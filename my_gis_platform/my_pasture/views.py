from django.shortcuts import render
from django.http import HttpResponse, HttpResponseRedirect
from django.template import loader
from .forms import InputForm
from .models import InputModel
from django.conf import settings
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
import urllib, base64
from django.http import JsonResponse


from sentinelhub import SHConfig
from sentinelhub import SentinelHubCatalog

import datetime, os, csv, math, io, matplotlib
from math import ceil
import matplotlib.pyplot as plt
import numpy as np
from shapely.ops import unary_union
from fiona.drvsupport import supported_drivers
import geopandas as gpd
from matplotlib.path import Path
from shapely.geometry import Polygon
import earthpy.spatial as es
import earthpy.plot as ep
import numpy.ma as ma
from scipy.stats.mstats import gmean, hmean
import pandas as pd
import seaborn as sns
from functools import reduce

matplotlib.use('agg')

from sentinelhub import (
    CRS,
    BBox,
    DataCollection,
    DownloadRequest,
    MimeType,
    MosaickingOrder,
    SentinelHubDownloadClient,
    SentinelHubRequest,
    bbox_to_dimensions,
    filter_times
)
sns.set()

# The following is not a package. It is a file utils.py which should be in the same folder as this notebook.
from utils import plot_image

colors = ['tomato', 'navy', 'MediumSpringGreen', 'lightblue', 'orange', 'blue',
          'maroon', 'purple', 'yellow', 'olive', 'brown', 'cyan']


class HolderClass():
    sentinel_request = None

class EvalScripts():

    def __init__(self, bands_dict, aux_data_dict):
        self.bands_dict = bands_dict
        self.aux_data_dict = aux_data_dict

        self.evalscript_true_color = """
            //VERSION=3

            function setup() {
                return {
                    input: [{
                        bands: ["B02", "B03", "B04"]
                    }],
                    output: {
                        bands: 3
                    }
                };
            }

            function evaluatePixel(sample) {
                return [sample.B02, sample.B03, sample.B04];
            }
        """

        self.evalscript_all_bands = """
            //VERSION=3
            function setup() {{
                return {{
                    input: [{{
                        bands: [{BANDS}],
                        units: "DN"
                    }}],
                    output: {{
                        bands: {COUNT},
                        sampleType: "INT16"
                    }}
                }};
            }}

            function evaluatePixel(sample) {{
                return [{SAMPLE}];
            }}
        """
        template1 = ""; template2 = ""
        for band in self.bands_dict.keys():
            template1 += f'"{band}", ';
            template2 += f'sample.{band}, ';
        settings = {"BANDS": template1, "SAMPLE": template2, "COUNT": str(len(self.bands_dict))}
        self.evalscript_all_bands = self.evalscript_all_bands.format(**settings)

        self.evalscript_aux_data = """
            //VERSION=3

            function setup() {{
                return {{
                    input: [{{
                        bands: [{BANDS}],
                        units: "DEGREES"
                    }}],
                    output: {{
                        bands: {COUNT},
                        sampleType: "FLOAT32"
                    }}
                }};
            }}

            function evaluatePixel(sample) {{
                return [{SAMPLE}];
            }}
        """
        template1 = ""; template2 = ""
        for band in self.aux_data_dict.keys():
            template1 += f'"{band}", ';
            template2 += f'sample.{band}, ';
        settings = {"BANDS": template1, "SAMPLE": template2, "COUNT": str(len(self.aux_data_dict))}
        self.evalscript_aux_data = self.evalscript_aux_data.format(**settings)



def normalize(band):
    band_min, band_max = (band.min(), band.max())
    return ((band - band_min)/((band_max - band_min)))

def brighten(band):
    alpha=0.13
    beta=0
    return np.clip(alpha*band+beta, 0, 255)



class SentinelRequest():

    def __init__(self, level, start_date, end_date, recent_date, kml_file):
        self.level = level
        self.start_date = start_date
        self.end_date = end_date
        self.recent_date = recent_date
        self.kml_file = kml_file.lstrip('/')
        self.project_path = settings.BASE_DIR

        self.CLIENT_ID = os.getenv('CLIENT_ID')
        self.CLIENT_SECRET = os.environ.get('CLIENT_SECRET')

        self.config = SHConfig()

        if self.CLIENT_ID and self.CLIENT_SECRET:
            self.config.sh_client_id = self.CLIENT_ID
            self.config.sh_client_secret = self.CLIENT_SECRET

        if not self.config.sh_client_id or not self.config.sh_client_secret:
            print("Warning! To use Process API, please provide the credentials (OAuth client ID and client secret).")

        self.catalog = SentinelHubCatalog(config=self.config)

        supported_drivers['KML'] = 'rw'
        self.pasture_df = gpd.read_file(self.kml_file, driver='KML')

        self.all_zagons = []
        for zagon in range(len(self.pasture_df.index)):
            self.all_zagons.append(self.pasture_df.loc[zagon].geometry)

        self.merged_zagons = unary_union(self.all_zagons)

        x_min, y_min, x_max, y_max = self.merged_zagons.bounds
        self.pasture_coords_wgs84 = (x_min, y_min, x_max, y_max)

        self.resolution = 10
        self.pasture_bbox = BBox(bbox=self.pasture_coords_wgs84, crs=CRS.WGS84)
        self.pasture_size = bbox_to_dimensions(self.pasture_bbox, resolution=self.resolution)

        self.data_collection = DataCollection.SENTINEL2_L2A if self.level == "L2A" else DataCollection.SENTINEL2_L1C
        self.bands_dict = dict([(band.name, i) for i, band in enumerate(self.data_collection.bands)])
        self.aux_data_dict = dict([(band, i) for i, band in enumerate(["sunZenithAngles","sunAzimuthAngles","viewZenithMean","viewAzimuthMean"])])

        self.scripts = EvalScripts(self.bands_dict, self.aux_data_dict)

        self.start_date_str = self.start_date.strftime("%Y-%m-%d")
        self.end_date_str = self.end_date.strftime("%Y-%m-%d")

        self.time_interval = self.start_date_str, self.end_date_str

        self.search_iterator = self.catalog.search(
            self.data_collection,
            bbox=self.pasture_bbox,
            time=self.time_interval,
            filter="eo:cloud_cover <= 100",
            fields={"include": ["id", "properties.datetime", "properties.eo:cloud_cover"], "exclude": []},
        )

        self.results = list(self.search_iterator)

        time_difference = datetime.timedelta(hours=1)

        all_timestamps = self.search_iterator.get_timestamps()
        self.unique_acquisitions = filter_times(all_timestamps, time_difference)



        self.true_color_process_requests = []

        for timestamp in self.unique_acquisitions:
            request = SentinelHubRequest(
                evalscript=self.scripts.evalscript_true_color,
                input_data=[
                    SentinelHubRequest.input_data(
                        data_collection=self.data_collection,
                        time_interval=(timestamp - time_difference, timestamp + time_difference),
                    )
                ],
                responses=[SentinelHubRequest.output_response("default", MimeType.PNG)],
                bbox=self.pasture_bbox,
                size=self.pasture_size,
                config=self.config,
            )
            self.true_color_process_requests.append(request)



        self.all_bands_process_requests = []

        for timestamp in self.unique_acquisitions:
            request = SentinelHubRequest(
                evalscript=self.scripts.evalscript_all_bands,
                input_data=[
                    SentinelHubRequest.input_data(
                        data_collection=self.data_collection,
                        time_interval=(timestamp - time_difference, timestamp + time_difference),
                    )
                ],
                responses=[SentinelHubRequest.output_response("default", MimeType.TIFF)],
                bbox=self.pasture_bbox,
                size=self.pasture_size,
                config=self.config,
            )
            self.all_bands_process_requests.append(request)




        self.aux_data_process_requests = []

        for timestamp in self.unique_acquisitions:
            request = SentinelHubRequest(
                evalscript=self.scripts.evalscript_aux_data,
                input_data=[
                    SentinelHubRequest.input_data(
                        data_collection=self.data_collection,
                        time_interval=(timestamp - time_difference, timestamp + time_difference),
                    )
                ],
                responses=[SentinelHubRequest.output_response("default", MimeType.TIFF)],
                bbox=self.pasture_bbox,
                size=self.pasture_size,
                config=self.config,
            )
            self.aux_data_process_requests.append(request)


        self.client = SentinelHubDownloadClient(config=self.config)

        true_color_download_requests = [request.download_list[0] for request in self.true_color_process_requests]
        self.true_color_data = self.client.download(true_color_download_requests)

        all_bands_download_requests = [request.download_list[0] for request in self.all_bands_process_requests]
        self.all_bands_data = self.client.download(all_bands_download_requests)

        aux_data_download_requests = [request.download_list[0] for request in self.aux_data_process_requests]
        self.aux_data = self.client.download(aux_data_download_requests)

        self.aoi_height, self.aoi_width, _ = self.true_color_data[-1].shape

        self.masks = []
        self.pasture_edges = []
        for zagon in range(len(self.pasture_df)-1):
            polygon=[]

            for coords in self.pasture_df.loc[zagon].geometry.exterior.coords:

                x = int(np.interp(coords[0], [x_min, x_max], [0, self.aoi_width]))
                y = int(np.interp(coords[1], [y_min, y_max], [self.aoi_height, 0]))

                polygon.append((y, x))

            poly_path=Path(polygon)
            x, y = np.mgrid[:self.aoi_height, :self.aoi_width]
            coors=np.hstack((x.reshape(-1, 1), y.reshape(-1,1)))

            self.pasture_edges.append(Polygon(polygon))

            mask = ~poly_path.contains_points(coors)
            self.masks.append(mask)

        self.combined_mask = reduce(np.logical_and, self.masks)

        white_noise_threshold = 255 # Значение [0-255]
        white_noise_count = self.aoi_width*self.aoi_height # Количество 157*78=[0-12246]

        self.clear_date_dict = []
        for i, timestamp in enumerate(self.unique_acquisitions):
            self.clear_date_dict.append((str(timestamp.date().isoformat()), i))
        self.clear_date_dict = dict(self.clear_date_dict)


    def get_unique_acquisitions(self):
        return self.clear_date_dict

    def set_choosen_date(self, pk):
        self.image_date = int(pk)

    def prepare_all_bands(self):
        self.ULTRA_BLUE = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B01"]]))

        self.BLUE = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B02"]]))
        self.GREEN = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B03"]]))
        self.RED = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B04"]]))

        self.RED_EDGE1 = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B05"]]))
        self.RED_EDGE2 = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B06"]]))
        self.RED_EDGE3 = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B07"]]))

        self.NIR = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B08"]]))
        self.N_NIR = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B8A"]]))
        self.WV = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B09"]]))
        if "B10" in self.bands_dict:
            self.SWIR_C = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B10"]]))
        self.SWIR2 = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B11"]]))
        self.SWIR3 = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B12"]]))

        self.SAA = (self.aux_data[self.image_date][:, :, self.aux_data_dict["sunAzimuthAngles"]]).mean()
        self.SZA = (self.aux_data[self.image_date][:, :, self.aux_data_dict["sunZenithAngles"]]).mean()
        self.VAM = (self.aux_data[self.image_date][:, :, self.aux_data_dict["viewAzimuthMean"]]).mean()
        self.VZM = (self.aux_data[self.image_date][:, :, self.aux_data_dict["viewZenithMean"]]).mean()

        self.precision = 4
        self.general_info = f"|| {self.unique_acquisitions[self.image_date].date().isoformat()} || SZA: {str(round(self.SZA, self.precision))}, VZA: {str(round(self.VZM, self.precision))} || Level: {self.data_collection.processing_level}"

    def get_true_color_image(self):

        fig, ax = plt.subplots(figsize=(12, 5))
        for zagon in range(len(self.pasture_df)-1):

            ax.plot(self.pasture_edges[zagon].exterior.xy[1], self.pasture_edges[zagon].exterior.xy[0])

        ep.plot_rgb(np.stack([self.RED, self.GREEN, self.BLUE]), ax=ax,
                    title=self.general_info,
                    figsize=(12, 5),
                    )

        fig.tight_layout()

        image_stream = io.BytesIO()
        fig.savefig(image_stream, format='png')
        image_stream.seek(0)
        encoded_image = base64.b64encode(image_stream.getvalue()).decode('utf-8')

        return encoded_image


    def get_requested_index(self, formula, relative_radio, absolute_radio, upper_bound, lower_bound, threshold_check, threshold):

        relative_radio = bool_converter(relative_radio)
        absolute_radio = bool_converter(absolute_radio)
        threshold_check = bool_converter(threshold_check)

        try:
            test_index = eval(modify_formula(formula))

            if threshold_check:
                test_thresh = float(threshold)
            else:
                test_thresh = test_index.min()

            test_filter = test_index >= test_thresh
            test_mask = ~test_filter
            test_meet = ma.masked_array(test_index, mask=test_mask)
            test_meet_pasture = ma.masked_array(test_meet, mask=self.combined_mask.reshape(self.aoi_height, self.aoi_width))

            if relative_radio:
                lower_bound = test_meet_pasture.min()
                upper_bound = test_meet_pasture.max()
            if absolute_radio:
                lower_bound = float(lower_bound)
                upper_bound = float(upper_bound)


            fig, ax = plt.subplots(figsize=(12, 5))
            for zagon in range(len(self.pasture_df)-1):

                ax.plot(self.pasture_edges[zagon].exterior.xy[1], self.pasture_edges[zagon].exterior.xy[0])

            header = formula
            self.precision = 4

            ep.plot_bands(test_meet_pasture, title=f"{header} {self.general_info}", ax=ax, cmap="bwr", cols=1, vmin=lower_bound, vmax=upper_bound, figsize=(10, 14))

            fig.tight_layout()


            image_stream = io.BytesIO()
            fig.savefig(image_stream, format='png')
            image_stream.seek(0)
            encoded_image = base64.b64encode(image_stream.getvalue()).decode('utf-8')

            return encoded_image

        except Exception as error:
            print(error)
            return



        # test_index_masked_array = []
        # for i, mask in enumerate(masks):
        #     mx = ma.masked_array(test_index, mask=mask.reshape(aoi_height, aoi_width))
        #     test_index_masked_array.append(mx)

        # summary_data = []
        # for i, zagon in enumerate(test_index_masked_array):
        #     ep.hist(zagon, colors = colors[i], title=f'{header} || Загон-{i+1} {general_info}', cols=4, alpha=0.5,
        #     figsize = (10, 6))
        #     print(f"Загон №{i+1} || Сумма: {round(zagon.sum()/zagon.count(),precision)} || Среднаяя: {round(zagon.mean(),precision)} || Медианная: {round(ma.median(zagon),precision)} || Макс: {round(zagon.max(),precision)} || Мин: {round(zagon.min(),precision)}")
        #     summary_data.append([f"№{i+1}", round(zagon.sum()/zagon.count(),precision), round(zagon.mean(),precision), round(ma.median(zagon),precision), round(zagon.max(),precision), round(zagon.min(),precision)])
        #     plt.axvline(test_index_masked_array[i].mean(), color='b', linestyle='dashed', linewidth=2)
        #     plt.axvline(ma.median(test_index_masked_array[i]), color='r', linestyle='dashed', linewidth=2)
        #     has_negative_or_zero = test_index_masked_array[i] <= 0
        #     if not has_negative_or_zero.sum():
        #         plt.axvline(hmean(test_index_masked_array[i].reshape(aoi_width * aoi_height)), color='g', linestyle='dashed', linewidth=2)
        #         plt.axvline(gmean(test_index_masked_array[i].reshape(aoi_width * aoi_height)), color='y', linestyle='dashed', linewidth=2)
        #         plt.legend([f"Средняя: {test_index_masked_array[i].mean()}",f"Медианная: {ma.median(test_index_masked_array[i])}",f"Гармоническая: {hmean(test_index_masked_array[i].reshape(aoi_width * aoi_height))}",f"Геометрическая: {gmean(test_index_masked_array[i].reshape(aoi_width * aoi_height))}"], title=f'Сумма: {round(zagon.sum(),precision)}')
        #     else:
        #         plt.legend([f"Средняя: {ma.mean(test_index_masked_array[i])}",f"Медианная: {ma.median(test_index_masked_array[i])}"], title=f'Сумма: {round(zagon.sum(),precision)}')
        # plt.show()

        # summary_df = pd.DataFrame(data = summary_data, columns=["Загон", "Сумма", "Cреднаяя", "Медианная", "Макс", "Мин"])
        # summary_df.to_excel(f"Summary_{date_chosen}_{data_collection.processing_level}.xlsx", index=None)




def dates_request(request):
    if request.method == "POST":

        form = InputForm(request.POST, request.FILES)

        if form.is_valid():
            level = form.cleaned_data["level"]
            start_date = form.cleaned_data["start_date"]
            end_date = form.cleaned_data["end_date"]
            recent_date = form.cleaned_data["recent_date"]
            kml_file = request.FILES["kml_file"]
            form.save()
            return HttpResponseRedirect("/available_dates")
    else:
        form = InputForm()

    return render(request, "my_pasture/pasture.html", {"form": form})



def about(request):
    return render(request, "my_pasture/about.html")


def available_dates(request):
    global sentinel_request

    last_object = InputModel.objects.latest('id')

    level = last_object.level
    start_date = last_object.start_date
    end_date = last_object.end_date
    recent_date = last_object.recent_date
    kml_file = last_object.kml_file.url

    HolderClass.sentinel_request = SentinelRequest(level, start_date, end_date, recent_date, kml_file)
    unique_acquisitions = HolderClass.sentinel_request.get_unique_acquisitions()
    context = {
        'unique_acquisitions': unique_acquisitions,
        'counter_start': 0,  # Starting value for the loop counter
    }
    return render(request, "my_pasture/available_dates.html", context)


def date_detail(request, *args, **kwargs):
    pk = request.POST.get('pk')

    HolderClass.sentinel_request.set_choosen_date(pk)
    HolderClass.sentinel_request.prepare_all_bands()
    image_data = HolderClass.sentinel_request.get_true_color_image()

    return render(request, 'my_pasture/date_detail.html', {'pk': pk, "image_data": image_data})


def modify_formula(formula):
    old_words = ["ULTRA_BLUE", "BLUE", "GREEN", "RED", "RED_EDGE1", "RED_EDGE2", "RED_EDGE3", "NIR", "N_NIR", "WV", "SWIR_C", "SWIR2", "SWIR3"]
    new_words = ["self.ULTRA_BLUE", "self.BLUE", "self.GREEN", "self.RED", "self.RED_EDGE1", "self.RED_EDGE2", "self.RED_EDGE3", "self.NIR", "self.N_NIR", "self.WV", "self.SWIR_C", "self.SWIR2", "self.SWIR3"]

    for old_word, new_word in zip(old_words, new_words):
        formula = formula.replace(old_word, new_word)
    return formula


def bool_converter(js_bool):
    if js_bool == "true":
        return True
    if js_bool == "false":
        return False


def pasture(index):
    only_pasture = ma.masked_array(ma.masked_array((index), mask=np.isinf((index)) | np.isnan((index))), mask=combined_mask.reshape(aoi_height, aoi_width))
    return only_pasture


def ajax_view(request):
    if request.META.get('HTTP_X_REQUESTED_WITH') == 'XMLHttpRequest' and request.method == 'GET':

        formula = request.GET.get('formula')
        relative_radio = request.GET.get('relative_radio')
        absolute_radio = request.GET.get('absolute_radio')
        upper_bound = request.GET.get('upper_bound')
        lower_bound = request.GET.get('lower_bound')
        threshold_check = request.GET.get('threshold_check')
        threshold = request.GET.get('threshold')

        index_image = HolderClass.sentinel_request.get_requested_index(formula, relative_radio, absolute_radio, upper_bound, lower_bound, threshold_check, threshold)

        response_data = {'index_image': index_image}

        return JsonResponse(response_data)
    else:
        return JsonResponse({'message': 'Invalid request'})

