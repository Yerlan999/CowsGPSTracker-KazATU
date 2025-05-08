from django.shortcuts import render
from django.http import HttpResponse, HttpResponseRedirect
from django.template import loader
from django.views.decorators.csrf import csrf_exempt
from .forms import InputForm
from .models import InputModel
from django.conf import settings
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
import urllib, base64
from django.http import JsonResponse
from pandas import json_normalize
from IPython.display import display

from twilio.rest import Client
from sklearn.preprocessing import StandardScaler
from keras.models import load_model
from scipy.ndimage import binary_dilation

from googletrans import Translator

import urllib.request
import urllib.error
import requests
import joblib
from bs4 import BeautifulSoup

from sentinelhub import SHConfig
from sentinelhub import SentinelHubCatalog

from datetime import timezone, timedelta
import datetime, os, csv, math, io, matplotlib, json, re, codecs, sys, random, itertools, statistics, pickle
from math import ceil
import matplotlib.pyplot as plt
import numpy as np
from shapely.ops import unary_union
from fiona.drvsupport import supported_drivers
import geopandas as gpd
from matplotlib.path import Path
from shapely.geometry import Polygon, MultiPolygon, Point, mapping

import earthpy.spatial as es
import earthpy.plot as ep
import numpy.ma as ma
from scipy.stats.mstats import gmean, hmean
import pandas as pd
import seaborn as sns
from functools import reduce

import tensorflow as tf

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


parameters_to_ignore = ["time", "sunrise", "sunset", "weathercode"]


FORECAST_PARAMETERS = [
    "temperature_2m_max",
    "temperature_2m_min",
    "apparent_temperature_max",
    "apparent_temperature_min",
    "precipitation_sum",
    "rain_sum",
    "showers_sum",
    "snowfall_sum",
    "precipitation_hours",
    "precipitation_probability_max",
    "precipitation_probability_min",
    "precipitation_probability_mean",
    "weathercode",
    "sunrise",
    "sunset",
    "windspeed_10m_max",
    "windgusts_10m_max",
    "winddirection_10m_dominant",
    "shortwave_radiation_sum",
    "et0_fao_evapotranspiration",
    "uv_index_max",
    "uv_index_clear_sky_max",]

FORECAST_RUSSIAN_PARAMETERS = [

    "температура_2м_макс",
    "температура_2м_мин",
    "температура_ощущ_макс",
    "температура_ощущ_мин",
    "осадки_сумма",
    "дождь_сумма",
    "ливни_сумма",
    "снегопад_сумма",
    "осадки_часы",
    "вероятность_осадков_макс",
    "вероятность_осадков_мин",
    "вероятность_осадков_средняя",
    "скорость_ветра_10м_макс",
    "порывы_ветра_10м_макс",
    "направление_ветра_10м",
    "коротковолновое_излучение_сумма",
    "эвапотранспирация",
    "ультрафиолетовый_индекс_макс",
    "ультрафиолетовый_индекс_чистого_неба_макс",



]
HISTORY_PARAMETERS = [
    "temperature_2m_max",
    "temperature_2m_min",
    "apparent_temperature_max",
    "apparent_temperature_min",
    "precipitation_sum",
    "rain_sum",
    "snowfall_sum",
    "precipitation_hours",
    "weathercode",
    "sunrise",
    "sunset",
    "windspeed_10m_max",
    "windgusts_10m_max",
    "winddirection_10m_dominant",
    "shortwave_radiation_sum",
    "et0_fao_evapotranspiration",]

LAST_MAIN_DISCRIPTION = ""

HISTORY_RUSSIAN_PARAMETERS = [
    "температура_2м_макс",
    "температура_2м_мин",
    "температура_ощущ_макс",
    "температура_ощущ_мин",
    "осадки_сумма",
    "дождь_сумма",
    "снегопад_сумма",
    "осадки_часы",
    "скорость_ветра_10м_макс",
    "порывы_ветра_10м_макс",
    "направление_ветра_10м",
    "коротковолновое_излучение_сумма",
    "эвапотранспирация",]



class HolderClass():
    sentinel_request = None

class EvalScripts():

    def __init__(self, bands_dict, aux_data_dict, universal_bands_dict):
        self.bands_dict = bands_dict
        self.aux_data_dict = aux_data_dict
        self.universal_bands_dict = universal_bands_dict

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

        if self.universal_bands_dict:
            self.universal_evalscript_all_bands = """
                //VERSION=3
                function setup() {{
                    return {{
                        input: [{{
                            bands: [{BANDS}],
                        }}],
                        output: {{
                            bands: {COUNT},
                        }}
                    }};
                }}

                function evaluatePixel(sample) {{
                    return [{SAMPLE}];
                }}
            """
            template1 = ""; template2 = ""
            for band in self.universal_bands_dict.keys():
                template1 += f'"{band}", ';
                template2 += f'sample.{band}, ';
            settings = {"BANDS": template1, "SAMPLE": template2, "COUNT": str(len(self.universal_bands_dict))}
            self.universal_evalscript_all_bands = self.universal_evalscript_all_bands.format(**settings)


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
        self.current_requested_index = None
        self.eng_column_names = ["temp", "feels_like", "pressure", "humidity", "dew_point", "clouds", "wind_speed", "wind_deg"]
        self.alt_eng_column_names = ["temp", "feels_like", "pressure", "humidity", "dew_point", "clouds_all", "wind_speed", "wind_deg"]
        self.rus_column_names = ["температура", "ощущается как", "атм. давление", "влажность воздуха", "точка росы", "облачность", "скорость ветра", "направление ветра"]
        self.precision = 4

        self.grand_history_weather_df = pd.read_csv('Pasture_Weather_History.csv')
        self.control_model = joblib.load('multi_target_rf_model.pkl')
        self.large_model = tf.keras.models.load_model('/large_model/')

        self.saved_mean = pd.read_pickle('saved_mean.pkl')
        self.saved_std = pd.read_pickle('saved_std.pkl')

        self.CLIENT_ID = os.getenv('CLIENT_ID')
        self.CLIENT_SECRET = os.environ.get('CLIENT_SECRET')
        self.WEATHERAPI = os.environ.get('OpenWeatherAPIKey')

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

        self.longitude = self.pasture_bbox.geometry.centroid.coords.xy[0][0]
        self.latitude = self.pasture_bbox.geometry.centroid.coords.xy[1][0]

        self.data_collection = DataCollection.SENTINEL2_L2A if self.level == "L2A" else DataCollection.SENTINEL2_L1C
        self.bands_dict = dict([(band.name, i) for i, band in enumerate(self.data_collection.bands)])
        self.aux_data_dict = dict([(band, i) for i, band in enumerate(["sunZenithAngles","sunAzimuthAngles","viewZenithMean","viewAzimuthMean"])])

        if self.level == "L2A":
            self.universal_bands_dict = dict([(band, i) for i, band in enumerate(["AOT", "SNW", "CLD", "SCL", "CLP", "CLM", "dataMask"])])
        else:
            self.universal_bands_dict = None

        self.scripts = EvalScripts(self.bands_dict, self.aux_data_dict, self.universal_bands_dict)

        self.start_date_str = self.start_date.strftime("%Y-%m-%d")
        self.end_date_str = self.end_date.strftime("%Y-%m-%d")

        self.time_interval = self.start_date_str, self.end_date_str

        # self.search_iterator = self.catalog.search(
        #     self.data_collection,
        #     bbox=self.pasture_bbox,
        #     time=self.time_interval,
        #     filter="eo:cloud_cover <= 100",
        #     fields={"include": ["id", "properties.datetime", "properties.eo:cloud_cover"], "exclude": []},
        # )


        # LOADINGS
        print("THE YEAR IS: ", self.start_date.year)

        with open(f'results_{self.start_date.year}.pkl', 'rb') as f:
            self.results = pickle.load(f)

        with open(f'uni_bands_data_{self.start_date.year}.pkl', 'rb') as f:
            self.uni_bands_data = pickle.load(f)

        with open(f'all_bands_data_{self.start_date.year}.pkl', 'rb') as f:
            self.all_bands_data = pickle.load(f)

        with open(f'aux_data_{self.start_date.year}.pkl', 'rb') as f:
            self.aux_data = pickle.load(f)

        with open(f'unique_acquisitions_{self.start_date.year}.pkl', 'rb') as f:
            self.unique_acquisitions = pickle.load(f)


        # self.results = list(self.search_iterator)

        # time_difference = datetime.timedelta(hours=1)

        # all_timestamps = self.search_iterator.get_timestamps()
        # self.unique_acquisitions = filter_times(all_timestamps, time_difference)

        # self.all_bands_process_requests = []

        # for timestamp in self.unique_acquisitions:
        #     request = SentinelHubRequest(
        #         evalscript=self.scripts.evalscript_all_bands,
        #         input_data=[
        #             SentinelHubRequest.input_data(
        #                 data_collection=self.data_collection,
        #                 time_interval=(timestamp - time_difference, timestamp + time_difference),
        #             )
        #         ],
        #         responses=[SentinelHubRequest.output_response("default", MimeType.TIFF)],
        #         bbox=self.pasture_bbox,
        #         size=self.pasture_size,
        #         config=self.config,
        #     )
        #     self.all_bands_process_requests.append(request)

        # self.aux_data_process_requests = []

        # for timestamp in self.unique_acquisitions:
        #     request = SentinelHubRequest(
        #         evalscript=self.scripts.evalscript_aux_data,
        #         input_data=[
        #             SentinelHubRequest.input_data(
        #                 data_collection=self.data_collection,
        #                 time_interval=(timestamp - time_difference, timestamp + time_difference),
        #             )
        #         ],
        #         responses=[SentinelHubRequest.output_response("default", MimeType.TIFF)],
        #         bbox=self.pasture_bbox,
        #         size=self.pasture_size,
        #         config=self.config,
        #     )
        #     self.aux_data_process_requests.append(request)


        # self.uni_bands_process_requests = []

        # if self.level == "L2A":
        #     for timestamp in self.unique_acquisitions:
        #         request = SentinelHubRequest(
        #             evalscript=self.scripts.universal_evalscript_all_bands,
        #             input_data=[
        #                 SentinelHubRequest.input_data(
        #                     data_collection=self.data_collection,
        #                     time_interval=(timestamp - time_difference, timestamp + time_difference),
        #                 )
        #             ],
        #             responses=[SentinelHubRequest.output_response("default", MimeType.TIFF)],
        #             bbox=self.pasture_bbox,
        #             size=self.pasture_size,
        #             config=self.config,
        #         )
        #         self.uni_bands_process_requests.append(request)


        # self.client = SentinelHubDownloadClient(config=self.config)

        # all_bands_download_requests = [request.download_list[0] for request in self.all_bands_process_requests]
        # self.all_bands_data = self.client.download(all_bands_download_requests)

        # aux_data_download_requests = [request.download_list[0] for request in self.aux_data_process_requests]
        # self.aux_data = self.client.download(aux_data_download_requests)

        # if self.level == "L2A":
        #     uni_bands_download_requests = [request.download_list[0] for request in self.uni_bands_process_requests]
        #     self.uni_bands_data = self.client.download(uni_bands_download_requests)




        self.aoi_height, self.aoi_width, _ = self.all_bands_data[-1].shape

        self.masks = []
        self.pasture_edges = []
        for zagon in range(len(self.pasture_df)):
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

        self.image_date_cloud = []
        for date in self.unique_acquisitions:
            for index in range(len(self.results)):
                if datetime.datetime.strptime(self.results[index]['properties']['datetime'], "%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=timezone.utc) == date:
                    self.image_date_cloud.append((str(date.date()), self.results[index]['properties']["eo:cloud_cover"]))
        self.image_date_cloud = dict(self.image_date_cloud)


    def get_unique_acquisitions(self):
        return list(zip(self.clear_date_dict, self.image_date_cloud.values()))

    def set_choosen_date(self, pk):
        self.image_date = int(pk)

    def set_date(self, date):
        self.image_date = self.clear_date_dict[date[0]]

    def norm(self, x):
        return (x - self.saved_mean) / self.saved_std

    def pasture(self, index):
        only_pasture = ma.masked_array(ma.masked_array((index), mask=np.isinf((index)) | np.isnan((index))), mask=self.combined_mask.reshape(self.aoi_height, self.aoi_width))
        return only_pasture

    def mean(self, index):
        return float(index.mean())

    def median(self, index):
        return float(ma.median(index))

    def get_zagons_values(self, index):
        test_index_masked_array = []
        for i, mask in enumerate(self.masks):
            mx = ma.masked_array(index, mask=mask.reshape(self.aoi_height, self.aoi_width))
            test_index_masked_array.append(mx)
        return test_index_masked_array

    def get_weather_parameters(self, date):

        current_date = datetime.datetime.now().strftime("%Y-%m-%d")

        longitude = HolderClass.sentinel_request.pasture_bbox.geometry.centroid.coords.xy[0][0]
        latitude = HolderClass.sentinel_request.pasture_bbox.geometry.centroid.coords.xy[1][0]

        api_key = self.WEATHERAPI  # Replace with your OpenWeatherMap API key
        date_object = datetime.datetime.strptime(date, "%Y-%m-%d")
        timestamp_utc = int(date_object.replace(tzinfo=timezone.utc).timestamp())

        current_date_datetime_object = datetime.datetime.strptime(current_date, '%Y-%m-%d')

        if not date_object == current_date_datetime_object:
            print("!!! From API for previous days !!!", date_object)
            url_template = "https://api.openweathermap.org/data/3.0/onecall/timemachine?lat={}&lon={}&dt={}&appid={}&units=metric"
            url = url_template.format(str(latitude), str(longitude), str(timestamp_utc), api_key)

            response = requests.get(url)

            if response.status_code == 200:
                data = response.json()
                current_weather_df = pd.concat([json_normalize(data["data"])], axis=1).drop(columns=["weather", "dt", "sunrise", "sunset"])
            else:
                print(f"Error: {response.status_code} - {response.text}")

        else:
            print("!!! From API for the current day !!!", date_object)

            url_template = "https://api.openweathermap.org/data/3.0/onecall?lat={}&lon={}&appid={}&units=metric"
            # url_template = "https://api.openweathermap.org/data/2.5/weather?lat={}&lon={}&appid={}&units=metric"

            url = url_template.format(str(latitude), str(longitude), api_key)

            response = requests.get(url)

            if response.status_code == 200:
                data = response.json()
                try:
                    current_weather_df = pd.concat([json_normalize(data["main"]), json_normalize(data["wind"]), pd.DataFrame({'visibility': data["visibility"]}, index=[0])], axis=1)
                except:
                    data["current"].pop('weather', None); current_weather_df = pd.DataFrame([data["current"]])
            else:
                print(f"Error: {response.status_code} - {response.text}")


        # !!! DO NOT DELETE THIS!!!
        # Getting from history file for a while...

        # try:
        #     print("!!! From File !!!")
        #     current_weather_df = self.get_main_weather_params_history(date).loc[:, self.eng_column_names]
        # except:
        #     print("!!! From File e !!!")
        #     current_weather_df = self.get_main_weather_params_history(date).loc[:, self.alt_eng_column_names]
        # !!! DO NOT DELETE THIS!!!

        try:
            current_weather_df = current_weather_df[['temp', 'feels_like', 'pressure', 'humidity', 'dew_point', 'clouds', 'wind_speed', 'wind_deg']]
        except:
            current_weather_df = current_weather_df[['temp', 'feels_like', 'pressure', 'humidity', 'speed', 'deg']]
            new_columns = pd.DataFrame(np.zeros((1, 2)), columns=['dew_point', 'clouds']); position = 4;
            current_weather_df = pd.concat([current_weather_df.iloc[:, :position], new_columns, current_weather_df.iloc[:, position:]], axis=1)


        current_weather_df.columns = self.rus_column_names
        self.current_weather_df = current_weather_df

        return self.current_weather_df


    def get_main_weather_params_history(self, seek_date):
        if isinstance(seek_date, list):
            l = []
            for d in seek_date:
                dt_object = datetime.datetime.strptime(d, "%Y-%m-%d")
                desired_timezone = datetime.timezone.utc
                result_datetime = dt_object.replace(hour=7, minute=0, second=0, tzinfo=desired_timezone)
                l.append(self.round_up_to_hour(result_datetime))
            date_list_df = pd.DataFrame({'dt_iso': l})
            df = pd.merge(self.grand_history_weather_df, date_list_df, on='dt_iso', how='right')
            df = df.drop(columns=['dt', 'dt_iso', 'timezone', 'city_name', 'lat', 'lon', 'weather_id', 'weather_main', 'weather_description', 'weather_icon'])
        else:
            dt_object = datetime.datetime.strptime(seek_date, "%Y-%m-%d")
            desired_timezone = datetime.timezone.utc
            result_datetime = dt_object.replace(hour=7, minute=0, second=0, tzinfo=desired_timezone)

            df = self.grand_history_weather_df[self.grand_history_weather_df["dt_iso"] == self.round_up_to_hour(result_datetime)]
            df = df.drop(columns=['dt', 'dt_iso', 'timezone', 'city_name', 'lat', 'lon', 'weather_id', 'weather_main', 'weather_description', 'weather_icon'])
        return df.reset_index(drop=True)

    def round_up_to_hour(self, timestamp):
        if timestamp.minute >= 30:
            # If minutes are above 30, round up to the next hour
            rounded_timestamp = timestamp.replace(
                minute=0, second=0, microsecond=0
            ) + timedelta(hours=1)
        else:
            # Otherwise, round down to the current hour
            rounded_timestamp = timestamp.replace(minute=0, second=0, microsecond=0)

        return rounded_timestamp.strftime('%Y-%m-%d %H:%M:%S %z %Z')

    def prepare_all_bands(self):

        self.FULL_BLUE = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B02"]]))
        self.FULL_GREEN = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B03"]]))
        self.FULL_RED = normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B04"]]))

        self.ULTRA_BLUE = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B01"]])))

        self.BLUE = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B02"]])))
        self.GREEN = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B03"]])))
        self.RED = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B04"]])))

        self.RED_EDGE1 = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B05"]])))
        self.RED_EDGE2 = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B06"]])))
        self.RED_EDGE3 = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B07"]])))

        self.NIR = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B08"]])))
        self.N_NIR = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B8A"]])))
        self.WV = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B09"]])))
        if "B10" in self.bands_dict:
            self.SWIR_C = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B10"]])))
        self.SWIR2 = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B11"]])))
        self.SWIR3 = self.pasture(brighten(normalize(self.all_bands_data[self.image_date][:, :, self.bands_dict["B12"]])))

        self.P_ULTRA_BLUE = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B01"]])))
        self.P_BLUE = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B02"]])))
        self.P_GREEN = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B03"]])))
        self.P_RED = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B04"]])))
        self.P_RED_EDGE1 = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B05"]])))
        self.P_RED_EDGE2 = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B06"]])))
        self.P_RED_EDGE3 = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B07"]])))
        self.P_NIR = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B08"]])))
        self.P_N_NIR = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B8A"]])))
        self.P_WV = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B09"]])))
        if "B10" in self.bands_dict:
            self.P_SWIR_C = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B10"]])))
        self.P_SWIR2 = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B11"]])))
        self.P_SWIR3 = brighten(normalize(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B12"]])))


        self.SAA = (self.aux_data[self.image_date][:, :, self.aux_data_dict["sunAzimuthAngles"]]).mean()
        self.SZA = (self.aux_data[self.image_date][:, :, self.aux_data_dict["sunZenithAngles"]]).mean()
        self.VAM = (self.aux_data[self.image_date][:, :, self.aux_data_dict["viewAzimuthMean"]]).mean()
        self.VZM = (self.aux_data[self.image_date][:, :, self.aux_data_dict["viewZenithMean"]]).mean()

        self.general_info = f"|| {self.unique_acquisitions[self.image_date].date().isoformat()} || SZA: {str(round(self.SZA, self.precision))}, VZA: {str(round(self.VZM, self.precision))} || Level: {self.data_collection.processing_level} || Облачность: {self.image_date_cloud[self.unique_acquisitions[self.image_date].date().isoformat()]}"

        if self.level == "L2A":
            self.cloud_mask = self.uni_bands_data[self.image_date][:, :, self.universal_bands_dict["CLD"]]/255
            self.cloud_mask = binary_dilation(self.cloud_mask, iterations=5)

    def get_true_color_image(self, size=(12, 5), with_title=True):

        fig, ax = plt.subplots(figsize=size)
        for zagon in range(len(self.pasture_df)):

            ax.plot(self.pasture_edges[zagon].exterior.xy[1], self.pasture_edges[zagon].exterior.xy[0])

        if with_title:
            ep.plot_rgb(np.stack([self.FULL_RED, self.FULL_GREEN, self.FULL_BLUE]), ax=ax,
                        title="RGB "+self.general_info,
                        figsize=size,
                        )
        else:
            ep.plot_rgb(np.stack([self.FULL_RED, self.FULL_GREEN, self.FULL_BLUE]), ax=ax,
                        figsize=size,
                        title="RGB "+self.general_info,
                        )
            ax.set_title("RGB "+self.general_info, fontsize=10)  # Adjust the font size as needed

        fig.tight_layout()

        image_stream = io.BytesIO()
        fig.savefig(image_stream, format='png')
        image_stream.seek(0)
        encoded_image = base64.b64encode(image_stream.getvalue()).decode('utf-8')

        return encoded_image

    def get_last2_true_color_images(self):
        current_image_id = self.image_date
        last_2_images = []; last_2_date_str = []; last_2_RZAs = [];
        for _ in range(2):
            self.image_date -= 1
            self.prepare_all_bands()
            last_2_RZAs.append(self.SZA + self.VZM)
            last_2_date_str.append(self.unique_acquisitions[self.image_date].date().isoformat());
            last_2_images.append(self.get_true_color_image())

        # Clearing
        self.image_date = current_image_id
        self.prepare_all_bands()
        return last_2_images, last_2_date_str, last_2_RZAs


    def get_last2_requested_index(self, formula, relative_radio, absolute_radio, upper_bound, lower_bound, threshold_check, threshold, operators, by_pasture, correction_coeff_able, correction_coeff):
        current_image_id = self.image_date
        mask_of_this_day = 1-self.cloud_mask

        last_2_indices = []; last_2_missing_parts = []; last_2_summary_df = []; last_2_geodataframe = []; str_dates = [];

        for _ in range(2):
            self.image_date -= 1
            self.prepare_all_bands()
            str_dates.append(self.unique_acquisitions[self.image_date].date().isoformat())

            try:
                test_index = eval(modify_formula(formula, by_pasture))

                if threshold_check:
                    test_thresh = float(threshold)
                    test_filter = eval("test_index" + str(operators) + "test_thresh")
                else:
                    test_thresh = test_index.min()
                    test_filter = test_index >= test_thresh

                test_mask = ~test_filter
                test_meet = ma.masked_array(test_index, mask=test_mask)

                if relative_radio:
                    lower_bound = test_meet.min()
                    upper_bound = test_meet.max()
                if absolute_radio:
                    lower_bound = float(lower_bound)
                    upper_bound = float(upper_bound)

                if bool(correction_coeff_able):
                    desired_median = float(correction_coeff)
                    current_median = ma.median(test_meet)
                    coefficient = desired_median / current_median
                    test_meet = test_meet * coefficient

                only_cloud_index = ma.masked_array(test_meet, mask=mask_of_this_day)
                clouds_values_by_zagons = self.get_zagons_values(only_cloud_index)

                fig, ax = plt.subplots(figsize=(12, 5))
                for zagon in range(len(self.pasture_df)):

                    ax.plot(self.pasture_edges[zagon].exterior.xy[1], self.pasture_edges[zagon].exterior.xy[0])

                header = formula

                ep.plot_bands(test_meet, title=f"{header} {self.general_info}", ax=ax, cmap="bwr", cols=1, vmin=lower_bound, vmax=upper_bound, figsize=(10, 14))
                fig.tight_layout()

                image_stream = io.BytesIO()
                fig.savefig(image_stream, format='png')
                image_stream.seek(0)
                encoded_image = base64.b64encode(image_stream.getvalue()).decode('utf-8')


                test_index_masked_array = []
                for i, mask in enumerate(self.masks):
                    mx = ma.masked_array(test_meet, mask=mask.reshape(self.aoi_height, self.aoi_width))
                    test_index_masked_array.append(mx)

                mx_pasture = ma.masked_array(test_meet, mask=self.combined_mask.reshape(self.aoi_height, self.aoi_width))
                test_index_masked_array.append(mx_pasture)

                summary_data = []; decoded_hist_images = [];
                for i, zagon in enumerate(test_index_masked_array):
                    if (i+1 == len(test_index_masked_array)): title = f'{header} || Пастбище {self.general_info}';
                    else: title = f'{header} || Загон-{i+1} {self.general_info}';

                    ep.hist(zagon, colors = colors[i], title=title, cols=4, alpha=0.5,
                    figsize = (12, 5))
                    if (i+1 < len(test_index_masked_array)):
                        summary_data.append([f"№{i+1}", round(zagon.sum(),self.precision), round(zagon.mean(),self.precision), round(ma.median(zagon),self.precision), round(zagon.max(),self.precision), round(zagon.min(),self.precision)])
                    plt.axvline(test_index_masked_array[i].mean(), color='b', linestyle='dashed', linewidth=2)
                    plt.axvline(ma.median(test_index_masked_array[i]), color='r', linestyle='dashed', linewidth=2)
                    has_negative_or_zero = test_index_masked_array[i] <= 0
                    if not has_negative_or_zero.sum():
                        plt.axvline(hmean(test_index_masked_array[i].reshape(self.aoi_width * self.aoi_height)), color='g', linestyle='dashed', linewidth=2)
                        plt.axvline(gmean(test_index_masked_array[i].reshape(self.aoi_width * self.aoi_height)), color='y', linestyle='dashed', linewidth=2)
                        plt.legend([f"Средняя: {test_index_masked_array[i].mean()}",f"Медианная: {ma.median(test_index_masked_array[i])}",f"Гармоническая: {hmean(test_index_masked_array[i].reshape(self.aoi_width * self.aoi_height))}",f"Геометрическая: {gmean(test_index_masked_array[i].reshape(self.aoi_width * self.aoi_height))}"], title=f'Сумма: {round(zagon.sum(),self.precision)}')
                    else:
                        plt.legend([f"Средняя: {ma.mean(test_index_masked_array[i])}",f"Медианная: {ma.median(test_index_masked_array[i])}"], title=f'Сумма: {round(zagon.sum(),self.precision)}')

                    image_buffer = io.BytesIO()
                    plt.savefig(image_buffer, format='png')
                    plt.close()
                    # Convert the image buffer to a base64-encoded string
                    image_encoded = base64.b64encode(image_buffer.getvalue()).decode('utf-8')
                    decoded_hist_images.append(image_encoded)


                paddocks_area = []
                for i, mask in enumerate(self.masks):
                    paddocks_area.append((mask.size-mask.sum())*(10**2)/10000)
                paddocks_area.append(sum(paddocks_area))

                summary_df = pd.DataFrame(data = summary_data, columns=["Загон", "Сумма", "Средняя", "Медианная", "Макс", "Мин"])
                sum_row = pd.DataFrame({'Загон': ["Пастбище"], 'Сумма': [round(float(summary_df['Сумма'].sum()),self.precision)], 'Средняя': [round(float(test_meet.mean()),self.precision)], 'Медианная': [round(float(ma.median(test_meet)),self.precision)], 'Макс': [summary_df['Макс'].max()], 'Мин': [summary_df['Мин'].min()]}, index=[len(summary_df.index)])
                summary_df = pd.concat([summary_df, sum_row])
                summary_df["Площадь"] = paddocks_area

                geodataframe = pd.concat([self.pasture_df, summary_df], axis=1)
                geodataframe = geodataframe.to_json(default=mapping)

                summary_df = summary_df.to_dict(orient='records')
                summary_df = json.dumps(summary_df, ensure_ascii=False)

                last_2_summary_df.append(summary_df); last_2_geodataframe.append(geodataframe);

                last_2_indices.append(encoded_image); last_2_missing_parts.append(clouds_values_by_zagons)

            except Exception as error:
                print()
                print("REASON:")
                print(error)
                return None, None

        self.image_date = current_image_id
        self.prepare_all_bands()
        str_dates.append(self.unique_acquisitions[self.image_date].date().isoformat())

        return last_2_indices, last_2_missing_parts, last_2_summary_df, last_2_geodataframe, str_dates


    def get_requested_index(self, formula, relative_radio, absolute_radio, upper_bound, lower_bound, threshold_check, threshold, operators, by_pasture, correction_coeff_able, correction_coeff):

        relative_radio = bool_converter(relative_radio)
        absolute_radio = bool_converter(absolute_radio)
        threshold_check = bool_converter(threshold_check)
        by_pasture = bool_converter(by_pasture)

        try:
            test_index = eval(modify_formula(formula, by_pasture))

            if self.level == "L2A":

                cloud_covered_index = ma.masked_array(test_index, mask=self.cloud_mask)
                only_cloud_index = ma.masked_array(test_index, mask=1-self.cloud_mask)

                # Calling for last 2 days history here
                last2_indices, last2_missing_parts, last_2_summary_df, last_2_geodataframe, str_dates = self.get_last2_requested_index(formula, relative_radio, absolute_radio, upper_bound, lower_bound, threshold_check, threshold, operators, by_pasture, correction_coeff_able, correction_coeff)

                clouds_values_by_zagons = self.get_zagons_values(only_cloud_index)
                clouds_masks_by_zagons = self.get_zagons_values(self.cloud_mask)
                test_index_masked_array = self.get_zagons_values(cloud_covered_index)

                for i, (zagon_cloud_mask, zagon_index, previous_day, the_day_before_previous) in enumerate(zip(clouds_masks_by_zagons, test_index_masked_array, last2_missing_parts[0], last2_missing_parts[1]), start=1):

                    prev_mean = previous_day.mean()
                    before_prev = the_day_before_previous.mean()
                    trend = prev_mean - before_prev
                    updated_part = previous_day + trend

                    mask = zagon_cloud_mask == 1.0
                    test_index[mask] = updated_part[mask]  # normal_distribution_values


            if threshold_check:
                test_thresh = float(threshold)
                test_filter = eval("test_index" + str(operators) + "test_thresh")
            else:
                test_thresh = test_index.min()
                test_filter = test_index >= test_thresh

            test_mask = ~test_filter
            test_meet = ma.masked_array(test_index, mask=test_mask)

            if relative_radio:
                lower_bound = test_meet.min()
                upper_bound = test_meet.max()
            if absolute_radio:
                lower_bound = float(lower_bound)
                upper_bound = float(upper_bound)


            if bool(correction_coeff_able):
                desired_median = float(correction_coeff)
                current_median = ma.median(test_meet)
                coefficient = desired_median / current_median
                test_meet = test_meet * coefficient

            fig, ax = plt.subplots(figsize=(12, 5))
            for zagon in range(len(self.pasture_df)):

                ax.plot(self.pasture_edges[zagon].exterior.xy[1], self.pasture_edges[zagon].exterior.xy[0])

            header = formula
            self.current_requested_index = test_meet

            ep.plot_bands(test_meet, title=f"{header} {self.general_info}", ax=ax, cmap="bwr", cols=1, vmin=lower_bound, vmax=upper_bound, figsize=(10, 14))

            fig.tight_layout()


            image_stream = io.BytesIO()
            fig.savefig(image_stream, format='png')
            image_stream.seek(0)
            encoded_image = base64.b64encode(image_stream.getvalue()).decode('utf-8')


            test_index_masked_array = []
            for i, mask in enumerate(self.masks):
                mx = ma.masked_array(test_meet, mask=mask.reshape(self.aoi_height, self.aoi_width))
                test_index_masked_array.append(mx)

            mx_pasture = ma.masked_array(test_meet, mask=self.combined_mask.reshape(self.aoi_height, self.aoi_width))
            test_index_masked_array.append(mx_pasture)

            summary_data = []; decoded_hist_images = [];
            for i, zagon in enumerate(test_index_masked_array):
                if (i+1 == len(test_index_masked_array)): title = f'{header} || Пастбище {self.general_info}';
                else: title = f'{header} || Загон-{i+1} {self.general_info}';

                ep.hist(zagon, colors = colors[i], title=title, cols=4, alpha=0.5,
                figsize = (12, 5))
                if (i+1 < len(test_index_masked_array)):
                    summary_data.append([f"№{i+1}", round(float(zagon.sum()),self.precision), round(float(zagon.mean()),self.precision), round(float(ma.median(zagon)),self.precision), round(float(zagon.max()),self.precision), round(float(zagon.min()),self.precision)])
                plt.axvline(test_index_masked_array[i].mean(), color='b', linestyle='dashed', linewidth=2)
                plt.axvline(ma.median(test_index_masked_array[i]), color='r', linestyle='dashed', linewidth=2)
                has_negative_or_zero = test_index_masked_array[i] <= 0
                if not has_negative_or_zero.sum():
                    plt.axvline(hmean(test_index_masked_array[i].reshape(self.aoi_width * self.aoi_height)), color='g', linestyle='dashed', linewidth=2)
                    plt.axvline(gmean(test_index_masked_array[i].reshape(self.aoi_width * self.aoi_height)), color='y', linestyle='dashed', linewidth=2)
                    plt.legend([f"Средняя: {test_index_masked_array[i].mean()}",f"Медианная: {ma.median(test_index_masked_array[i])}",f"Гармоническая: {hmean(test_index_masked_array[i].reshape(self.aoi_width * self.aoi_height))}",f"Геометрическая: {gmean(test_index_masked_array[i].reshape(self.aoi_width * self.aoi_height))}"], title=f'Сумма: {round(zagon.sum(),self.precision)}')
                else:
                    plt.legend([f"Средняя: {ma.mean(test_index_masked_array[i])}",f"Медианная: {ma.median(test_index_masked_array[i])}"], title=f'Сумма: {round(zagon.sum(),self.precision)}')

                image_buffer = io.BytesIO()
                plt.savefig(image_buffer, format='png')
                plt.close()
                # Convert the image buffer to a base64-encoded string
                image_encoded = base64.b64encode(image_buffer.getvalue()).decode('utf-8')
                decoded_hist_images.append(image_encoded)


            paddocks_area = []
            for i, mask in enumerate(self.masks):
                paddocks_area.append((mask.size-mask.sum())*(10**2)/10000)
            paddocks_area.append(sum(paddocks_area))

            summary_df = pd.DataFrame(data = summary_data, columns=["Загон", "Сумма", "Средняя", "Медианная", "Макс", "Мин"])
            sum_row = pd.DataFrame({'Загон': ["Пастбище"], 'Сумма': [round(float(summary_df['Сумма'].sum()),self.precision)], 'Средняя': [round(float(test_meet.mean()),self.precision)], 'Медианная': [round(float(ma.median(test_meet)),self.precision)], 'Макс': [summary_df['Макс'].max()], 'Мин': [summary_df['Мин'].min()]}, index=[len(summary_df.index)])
            summary_df = pd.concat([summary_df, sum_row])
            summary_df["Площадь"] = paddocks_area

            geodataframe = pd.concat([self.pasture_df, summary_df], axis=1)
            geodataframe = geodataframe.to_json(default=mapping)

            summary_df = summary_df.to_dict(orient='records')
            summary_df = json.dumps(summary_df, ensure_ascii=False)

            centroid = self.merged_zagons.centroid.x, self.merged_zagons.centroid.y

            return encoded_image, decoded_hist_images, summary_df, geodataframe, centroid, last2_indices, last_2_summary_df, last_2_geodataframe, str_dates

        except Exception as error:
            print()
            print("REASON:")
            print(error)
            return None, None, None, None, None, None

    def get_weather_data(self, date):
        return self.get_weather_parameters(date)



def dates_request(request):
    if request.method == "POST":
        form = InputForm(request.POST, request.FILES)
        if form.is_valid():
            instance = form.save(commit=False)
            if instance.recent_date:
                instance.start_date = datetime.date.today() - datetime.timedelta(days=9)
                instance.end_date = datetime.date.today()
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

    dict_of_images = dict()
    for date in unique_acquisitions:
        HolderClass.sentinel_request.set_date(date)
        HolderClass.sentinel_request.prepare_all_bands()
        image_data = HolderClass.sentinel_request.get_true_color_image(size=(6, 3), with_title=False)
        dict_of_images[date] = image_data

    context = {
        'dict_of_images': dict_of_images,
        'unique_acquisitions': unique_acquisitions,
        'counter_start': 0,  # Starting value for the loop counter
    }
    return render(request, "my_pasture/available_dates.html", context)


def date_detail(request, *args, **kwargs):
    pk = request.POST.get('pk')

    HolderClass.sentinel_request.set_choosen_date(pk)
    HolderClass.sentinel_request.prepare_all_bands()
    image_data = HolderClass.sentinel_request.get_true_color_image()
    previous_2_days, last_2_date_str, last_2_RZAs = HolderClass.sentinel_request.get_last2_true_color_images()

    dict_of_dates = HolderClass.sentinel_request.clear_date_dict
    date_string = next((key for key, value in dict_of_dates.items() if value == int(pk)), None)

    all_3_date_str = [date_string] + last_2_date_str
    RZA = HolderClass.sentinel_request.SZA + HolderClass.sentinel_request.VZM

    weather_df1 = HolderClass.sentinel_request.get_weather_data(date_string)
    weather_html_df1 = weather_df1.to_html(classes='table table-striped table-bordered', escape=False, index=False, table_id='weather_table')
    weather_html_df1 = weather_html_df1.replace('<thead>', '<thead class="thead-dark">')

    other_2_previous_weather_df = []
    for i, date_string in enumerate(last_2_date_str, start=1):
        weather_df = HolderClass.sentinel_request.get_weather_data(date_string)
        weather_html_df = weather_df.to_html(classes='table table-striped table-bordered', escape=False, index=False, table_id=f'weather_table{i}')
        weather_html_df = weather_html_df.replace('<thead>', '<thead class="thead-dark">')
        other_2_previous_weather_df.append(weather_html_df)

    return render(request, 'my_pasture/date_detail.html', {'pk': pk, "image_data": image_data, "weather_data": weather_html_df1, "RZA": RZA, "previous_2_days": previous_2_days, "other_2_previous_weather_df": other_2_previous_weather_df, "last_2_RZAs": last_2_RZAs, "all_3_date_str": all_3_date_str})


def modify_formula(formula, by_pasture):

    if by_pasture:
        updated_formula = re.sub(r'\b(ULTRA_BLUE|BLUE|GREEN|RED|RED_EDGE1|RED_EDGE2|RED_EDGE3|NIR|N_NIR|WV|SWIR_C|SWIR2|SWIR3|mean|median)\b', r'self.P_\1', formula)
    else:
        updated_formula = re.sub(r'\b(ULTRA_BLUE|BLUE|GREEN|RED|RED_EDGE1|RED_EDGE2|RED_EDGE3|NIR|N_NIR|WV|SWIR_C|SWIR2|SWIR3|mean|median)\b', r'self.\1', formula)

    return updated_formula


def bool_converter(js_bool):
    if js_bool == "true":
        return True
    if js_bool == "false":
        return False



def apply_params_to_URL(URL, parameters):
    URL += "&daily="
    for i, parameter in enumerate(parameters):
        if i < len(parameters) - 1:
            URL += parameter + ","
        else:
            URL += parameter
    URL += "&timezone=auto"

    return URL

def make_API_request(URL):
    try:
        # Convert from bytes to text
        resp_text = urllib.request.urlopen(URL).read().decode('UTF-8')
        # Use loads to decode from text
        json_obj = json.loads(resp_text)
        return json_obj
    except urllib.error.HTTPError  as e:
        ErrorInfo= e.read().decode()
        print('Error code: ', e.code, ErrorInfo)
        sys.exit()
    except  urllib.error.URLError as e:
        ErrorInfo= e.read().decode()
        print('Error code: ', e.code,ErrorInfo)
        sys.exit()


def get_weather_mapping():
    global LAST_MAIN_DISCRIPTION

    url = "https://www.nodc.noaa.gov/archive/arc0021/0002199/1.1/data/0-data/HTML/WMO-CODE/WMO4677.HTM"
    response = requests.get(url)
    soup = BeautifulSoup(response.text, 'html.parser')

    weather_mapping = {}

    for i_table, table in enumerate(soup.select('table[BORDER]')):
        for i_row, row in enumerate(table.find_all('tr')[1:]):
            code_and_description = row.find_all('td')[:2]
            code = int(code_and_description[0].text.strip())
            description = code_and_description[1].text.strip()
            if not description.startswith("-"):
                LAST_MAIN_DISCRIPTION = description
            else:
                description = LAST_MAIN_DISCRIPTION
            weather_mapping[code] = description


    return weather_mapping


breed_factors = {
    "Angus": 0.025,
    "Hereford": 0.026,
    "Limousin": 0.028,
    "Charolais": 0.029,
}

def estimated_daily_weight_gain(initial_weight, age_in_months, breed_type):
    A = 0.025
    B = int(np.interp(age_in_months, [12, 24], [20, 80]))
    C = breed_factors.get(breed_type, 0.027)
    DWG = A * (initial_weight - B) * C
    return round(DWG, 4)

def dry_matter_intake(initial_weight, breed_type):
    D = breed_factors.get(breed_type, 0.027)
    DMI = D * initial_weight
    return round(DMI, 4)


class TimeLine:
    watcher = dict()

class Cattle():
    cattle_count = 0
    cattle_stack = []

    def __init__(self, sex, weight, stage=None, breed="Angus", production="beef", age=12, forage_to_gain=0.25):
        self.breed = breed           # Порода
        self.sex = sex               # Пол: Корова/Бык
        self.production = production # Направление: Молочное/Мясное
        self.age = age               # Возраст
        self.stage = stage           # Стадия/период
        self.weight = weight         # Начальный вес

        self.current_paddock = None
        self.day_entered = None

        self.gps_coordinate = None

        self.already_consumed_forage = 0
        self.forage_needed = None
        self.forage_to_gain = forage_to_gain # 1 кг сухой массы = 0.25 кг прирост в массе

        self.forage_needed = dry_matter_intake(self.weight, self.breed)

        Cattle.cattle_count += 1
        Cattle.cattle_stack.append(self)
        self.index = Cattle.cattle_count

        self.weight_history = []
        self.forage_needed_history = []

    def update_age(self, grazing_day):
        if grazing_day % 30 == 0:
            self.age += 1

    def update_weight(self):
        weight_gain = estimated_daily_weight_gain(self.weight, self.age, self.breed)
        self.weight += weight_gain

    def update_forage_consumption(self):
        self.forage_needed = dry_matter_intake(self.weight, self.breed)



    def __str__(self):
        return f'Cattle №{self.index}. Already consumed: {self.already_consumed_forage} kg. In the paddock №{self.current_paddock} right now.'
    def __repr__(self):
        return f'Cattle №{self.index}. Already consumed: {self.already_consumed_forage} kg. In the paddock №{self.current_paddock} right now.'

    def enter_into_paddock(new_paddock_number):
        self.current_paddock = new_paddock_number
        self.day_entered = datetime.datetime.today()


    def get_gps_coordinate_request(self):
        # Making a request to intermediate module of the system
        # Obtaining GPS coordinates of an interested cattle
        self.gps_coordinate = (0, 0)
        return self.gps_coordinate

    def get_cattle_current_gps_coordinates(self):
        gps_coordinate = self.get_gps_coordinate_request()
        curr_coord_y, curr_coord_x = gps_coordinate # (Latitude, Longitude) = (54.216885, 69.517046)
        x = int(np.interp(curr_coord_x, [x_min, x_max], [0, aoi_width])) # 157
        y = int(np.interp(curr_coord_y, [y_min, y_max], [aoi_height, 0])) # 78

        for i, paddock in enumerate(masks, start=1):
            if not paddock.reshape(aoi_height, aoi_width)[y, x]:
                self.current_paddock = i
            else:
                self.current_paddock = None


def growth_rate_temperature(temp, mean=20, std_dev=20):
    exponent = -((temp - mean) ** 2) / (2 * std_dev ** 2)
    growth_rate = np.exp(exponent)
    return growth_rate

def growth_rate_precipitation(precip, mean=100, std_dev=30.0):
    exponent = -((precip - mean) ** 2) / (2 * std_dev ** 2)
    growth_rate = np.exp(exponent)
    return growth_rate

def growth_rate_radiation(radiation, mean=200.0, std_dev=50.0):
    exponent = -((radiation - mean) ** 2) / (2 * std_dev ** 2)
    growth_rate = np.exp(exponent)
    return growth_rate

class Plant():

    def __init__(self, name, growth_rate, death_rate,
                 temperature_range, moisture_range, radiation_range,
                 season_start_date = "2023-03-01",
                 season_end_date = "2023-09-30",
                 first_peak = "2023-04-01", max1_y = 0.9,
                 second_peak = "2023-08-20", max2_y = 0.6,
                 sigma = 50,
                 plant_density = 150, # кг/м3
                ):
        self.name = name

        self.growth_rate = growth_rate # мм/день
        self.death_rate = death_rate   # мм/день

        self.temperature_range = temperature_range
        self.moisture_range = moisture_range
        self.radiation_range = radiation_range
        self.score_range = input_values = [0, 1, 0]

        self.max1_y = max1_y
        self.max2_y = max2_y
        self.sigma = sigma
        self.plant_density = plant_density

        self.season_start_date = datetime.datetime.strptime(season_start_date, "%Y-%m-%d")
        self.season_end_date = datetime.datetime.strptime(season_end_date, "%Y-%m-%d")
        self.season_duration = self.season_end_date - self.season_start_date

        self.first_peak = datetime.datetime.strptime(first_peak, "%Y-%m-%d")
        self.second_peak = datetime.datetime.strptime(second_peak, "%Y-%m-%d")


    def get_condition_score(self, grazing_day):
        start_date = grazing_day + datetime.timedelta(-15)
        end_date = grazing_day + datetime.timedelta(+15)

        filtered_df = Plant.history_df[(Plant.history_df['time'] >= str(start_date)) & (Plant.history_df['time'] <= str(end_date))]

        temperature = np.array((filtered_df['temperature_2m_max'] + filtered_df['temperature_2m_min']) / 2)
        precipitation = np.array(filtered_df['precipitation_sum'])
        radiation = np.array(filtered_df['shortwave_radiation_sum'])

        temperature_score = growth_rate_temperature(temperature, self.temperature_range[0], self.temperature_range[1])
        precipitation_score = growth_rate_precipitation(precipitation, self.moisture_range[0], self.moisture_range[1])
        radiation_score = growth_rate_radiation(radiation, self.radiation_range[0], self.radiation_range[1])

        return np.array([temperature_score, precipitation_score, radiation_score]).mean()


    def get_seasonal_score(self, grazing_day):
        days_passed = (grazing_day - self.season_start_date).days

        self.max1_x = (self.first_peak - self.season_start_date).days  # x value of the first maximum point
        self.max2_x = (self.second_peak - self.season_start_date).days  # x value of the second maximum point

        self.x = np.linspace(0, self.season_duration.days, self.season_duration.days)  # Adjust the range as needed

        self.sigma = 50  # Increase this value to make the curve less steep

        self.y = self.max1_y * np.exp(-((self.x - self.max1_x) / self.sigma) ** 2) + self.max2_y * np.exp(-((self.x - self.max2_x) / self.sigma) ** 2)

        return self.y[days_passed]


class Soil():

    def __init__(self, soil_type, humus_content, acidity, potassium_content, phosphorus_content, nitrogen_content):
        self.soil_type = soil_type
        self.humus_content = humus_content
        self.acidity = acidity
        self.potassium_content = potassium_content
        self.phosphorus_content = phosphorus_content
        self.nitrogen_content = nitrogen_content



class Grazing():

    def __init__(self,  start_date, end_date, pasture, num_rounds=2):
        self.pasture = pasture
        self.num_rounds = num_rounds
        self.start_date = datetime.datetime.strptime(start_date, "%Y-%m-%d")
        self.end_date = datetime.datetime.strptime(end_date, "%Y-%m-%d")
        self.grazing_duration = self.end_date - self.start_date
        self.grazing_cycle = itertools.cycle(range(1, len(self.pasture.paddock_masks)+1))
        value = next(self.grazing_cycle)
        setattr(self.pasture, "grazing_cycle", self.grazing_cycle)
        setattr(self.pasture, "start_date", self.start_date)
        setattr(self.pasture, "end_date", self.end_date)
        setattr(self.pasture, "grazing_duration", self.grazing_duration)
        self.preload_weather_df()

    def preload_weather_df(self):
        start_date = self.start_date + datetime.timedelta(-20)
        end_date = self.end_date + datetime.timedelta(+20)

        longitude = HolderClass.sentinel_request.pasture_bbox.geometry.centroid.coords.xy[0][0]
        latitude = HolderClass.sentinel_request.pasture_bbox.geometry.centroid.coords.xy[1][0]

        Hist_URL = f"https://archive-api.open-meteo.com/v1/archive?latitude={latitude}&longitude={longitude}&start_date={start_date.date()}&end_date={end_date.date()}"
        Hist_URL = apply_params_to_URL(Hist_URL, HISTORY_PARAMETERS)
        history_json_obj = make_API_request(Hist_URL)
        history_df = pd.DataFrame(history_json_obj["daily"])
        history_df.drop(columns=["weathercode", "sunrise", "sunset"], inplace=True)
        Plant.history_df = history_df

    def start_grazing(self):
        print("Grazing started!")
        for day in range(self.grazing_duration.days):
            current_date = self.start_date + datetime.timedelta(days=day)
#             print("Date: ", current_date.date())
            self.pasture.update_paddock_resource(current_date)
        print("Grazing finished!")


class Pasture():

    def __init__(self, size, paddock_masks):
        self.size = size
        self.paddock_masks = paddock_masks
        self.currently_grazed_paddock = None

        self.create_paddocks()

    def botanical_composition(self, paddock, plants_dict):
        if sum([percentage[1] for percentage in plants_dict.values()]) > 100:
            raise TypeError(f"100 < {sum([percentage[1] for percentage in plants_dict.values()])}")
            return
        setattr(self, f"paddock_{paddock}_plants", plants_dict)

    def set_paddock_resource(self, paddock, resource):
        setattr(self, f"paddock_{paddock}_resource", resource)

    def update_paddock_resource(self, grazing_day):
        for paddock, mask in enumerate(self.paddock_masks, start=1):
            plants_dict = getattr(self, f"paddock_{paddock}_plants")
            daily_forage_consumption = self.get_paddocks_daily_forage_consumption(paddock, grazing_day)

            daily_green_mass_delta = 0
            for plant, [obj, percentage] in plants_dict.items():
                number_of_squares = round(sum(~self.paddock_masks[paddock-1])*percentage/100)
                GI = obj.get_condition_score(grazing_day)
                SI = obj.get_seasonal_score(grazing_day)
                coeff = (GI+SI)/2; inv_coeff = 1-(GI+SI)/2
                green_mass_daily_increment = (random.uniform(10*coeff,10)*random.uniform(10*coeff,10))*(coeff*obj.growth_rate/1000)*obj.plant_density; # kg
                green_mass_daily_decrement = (random.uniform(10*inv_coeff,10)*random.uniform(10*inv_coeff,10))*(inv_coeff*obj.death_rate/1000)*obj.plant_density; # kg
                daily_green_mass_delta += (green_mass_daily_increment - green_mass_daily_decrement)

            paddock_resource = getattr(self, f"paddock_{paddock}_resource")
            TimeLine.watcher[f"paddock_{paddock}_resource_history"].append(paddock_resource)
            paddock_resource = round(paddock_resource + (daily_green_mass_delta - daily_forage_consumption))

            self.check_current_paddock_resource(self.currently_grazed_paddock)

            self.set_paddock_resource(paddock, paddock_resource)

    def check_current_paddock_resource(self, paddock_number):
        paddock_current_resource = getattr(self, f"paddock_{paddock_number}_resource")
        first_paddock_resource = TimeLine.watcher[f"paddock_{paddock_number}_resource_history"][0]
        if first_paddock_resource * 0.3 > paddock_current_resource:
            next_paddock_number = next(self.grazing_cycle)
            for cattle in Cattle.cattle_stack:
                self.add_cattle_into_paddock(cattle, next_paddock_number)


    def create_paddocks(self):
        for i, _ in enumerate(self.paddock_masks, start=1):
            setattr(self, f"paddock_{i}", [])
            TimeLine.watcher[f"paddock_{i}_resource_history"] = []

    def add_cattle_into_paddock(self, cattle, paddock_number):

        if cattle.current_paddock != paddock_number:
            if cattle.current_paddock:
                old_paddock = getattr(self, f"paddock_{cattle.current_paddock}")
                updated_old_paddock = [ocattle for ocattle in old_paddock if ocattle["object"].index != cattle.index]
                setattr(self, f"paddock_{cattle.current_paddock}", updated_old_paddock)

            cattle.current_paddock = paddock_number
            cattle_dict = dict();
            cattle_dict["id"] = cattle.index
            cattle_dict["object"] = cattle
            cattle_dict["paddock_number"] = cattle.current_paddock
            cattle_dict["forage_needed"] = cattle.forage_needed
            cattle_dict["already_consumed_forage"] = cattle.already_consumed_forage

            new_paddock = getattr(self, f"paddock_{paddock_number}")
            new_paddock.append(cattle_dict)
            self.currently_grazed_paddock = paddock_number
        else:
            print(f"{cattle} was sent to the same paddock!")

    def get_paddocks_daily_forage_consumption(self, paddock_number, grazing_day):
        total_forage_daily_consumption = 0
        for cattle in getattr(self, f"paddock_{paddock_number}"):
            total_forage_daily_consumption += cattle["object"].forage_needed

            cattle["object"].update_age((grazing_day - self.start_date).days)
            cattle["object"].update_weight()
            cattle["object"].update_forage_consumption()

            cattle["object"].weight_history.append(cattle["object"].weight)
            cattle["object"].forage_needed_history.append(cattle["object"].forage_needed)

        return total_forage_daily_consumption


test_cattle = {"latitude": 0,
                "longitude": 0,
                "index": 0,
                "paddock_number": 0}
pasture_load = None



# Function to randomize latitude and longitude
def randomize_coordinates(lat, lon, max_offset=0.000100):
    offset = random.uniform(-max_offset, max_offset)
    return lat + offset, lon + offset


def str2date(date_string):
    return datetime.datetime.strptime(date_string, "%Y-%m-%d").date()


def play_simulation(request):

    dict_of_pants = dict()

    plants_count = int(request["plants_count"])
    paddocks_count = int(request["paddocks_count"])

    pasture = Pasture(HolderClass.sentinel_request.pasture_size, HolderClass.sentinel_request.masks)


    for plant_id in range(plants_count):
        plant_name = request[f"plant{plant_id}"]

        temperatureToleranceIdeal = float(request[f"temperatureToleranceIdeal{plant_id}"])
        temperatureStandDev = float(request[f"temperatureStandDev{plant_id}"])

        moistureToleranceIdeal = float(request[f"moistureToleranceIdeal{plant_id}"])
        moistureStandDev = float(request[f"moistureStandDev{plant_id}"])

        radiationToleranceIdeal = float(request[f"radiationToleranceIdeal{plant_id}"])
        radiationStandDev = float(request[f"radiationStandDev{plant_id}"])

        growthRate = float(request[f"growthRate{plant_id}"])
        deathRate = float(request[f"deathRate{plant_id}"])
        firstPeak = request[f"firstPeak{plant_id}"]
        secondPeak = request[f"secondPeak{plant_id}"]
        max1Y = float(request[f"max1Y{plant_id}"])
        max2Y = float(request[f"max2Y{plant_id}"])
        sigma = float(request[f"sigma{plant_id}"])
        plantDensity = float(request[f"plantDensity{plant_id}"])

        temperatures = [temperatureToleranceIdeal, temperatureStandDev]
        moistures = [moistureToleranceIdeal, moistureStandDev]
        radiations = [radiationToleranceIdeal, radiationStandDev]

        plant_object = Plant(plant_name, growthRate, deathRate, temperatures, moistures, radiations,
                         first_peak = firstPeak, max1_y = max1Y,
                         second_peak = secondPeak, max2_y = max2Y,
                         sigma = sigma,
                         plant_density = plantDensity)

        dict_of_pants[plant_name] = plant_object

    for paddock in range(1, paddocks_count+1):
        paddock_content_dict = dict()
        paddock_resource = float(request[f"paddock_resource{paddock}"])
        for plant in range(plants_count):
            plant_name = request[f"plantsOptions-{paddock}-{plant}"][0]
            to_include = bool(request[f"plantsOptions-{paddock}-{plant}"][1])
            coverage = request[f"percentage-{paddock}-{plant}"]
            if to_include and coverage:
                paddock_content_dict[plant_name] = [dict_of_pants[plant_name], int(coverage)]

        pasture.botanical_composition(paddock, paddock_content_dict)
        paddock_size_ha = (HolderClass.sentinel_request.masks[paddock-1].size-HolderClass.sentinel_request.masks[paddock-1].sum())*(10**2)/10000
        pasture.set_paddock_resource(paddock, paddock_resource*1000*paddock_size_ha)

    cattle_count = int(request["cattleCount"])
    cattle_sex = request["cattleSex"]
    cattle_breed = request["cattleBreed"]
    lower_age = int(request["lowerAge"])
    upper_age = int(request["upperAge"])
    lower_weight = int(request["lowerWeight"])
    upper_weight = int(request["upperWeight"])

    if cattle_sex == "random":
        cattle_sex = ["male", "female"]
    else:
        cattle_sex = [cattle_sex]

    if cattle_breed == "random":
        cattle_breed = list(breed_factors.keys())
    else:
        cattle_breed = [cattle_breed]

    for _ in range(cattle_count):
        cattle = Cattle(sex=random.choice(cattle_sex), breed=random.choice(cattle_breed), weight=random.randint(lower_weight, upper_weight), age=random.randint(lower_age, upper_age))
        pasture.add_cattle_into_paddock(cattle, 1)

    grazing_start = request["grazingStart"]
    grazing_end = request["grazingEnd"]

    grazing = Grazing(grazing_start, grazing_end, pasture)
    grazing.start_grazing()

    paddocks_grazing_graphs = []
    for paddock in range(1, paddocks_count+1):

        data = TimeLine.watcher[f"paddock_{paddock}_resource_history"]

        indices = list(range(1, len(data) + 1))

        custom_x_ticks = []

        current_date = grazing.start_date
        while current_date <= grazing.end_date+datetime.timedelta(days=30):
            custom_x_ticks.append(current_date.strftime("%b"))
            current_date = current_date + datetime.timedelta(days=30)  # Approximate number of days in a month

        custom_x_ticks = list(set(custom_x_ticks))

        custom_x_ticks.sort(key=lambda x: datetime.datetime.strptime(x, "%b").month)
        custom_x_tick_positions = np.linspace(0, grazing.grazing_duration.days, len(custom_x_ticks))

        plt.figure(figsize=(10, 5))
        plt.xticks(custom_x_tick_positions, custom_x_ticks)

        plt.plot(indices, data, color="green")
        plt.xlabel("Дни")
        plt.ylabel(f"Урожайность (кг)")
        plt.title(f"Изменение урожайности в загоне №{paddock}.")
        plt.grid(True)

        image_buffer = io.BytesIO()
        plt.savefig(image_buffer, format='png')
        plt.close()
        image_encoded = base64.b64encode(image_buffer.getvalue()).decode('utf-8')
        paddocks_grazing_graphs.append(image_encoded)

    return paddocks_grazing_graphs


MODEL_COLUMNS = ["index", "RZM", "temp", "pressure", "humidity", "cattle_count", "daily_intake", "reserve"]


def send_SMS_message(message_content):
    account_sid = os.environ.get('Twilio_SID')
    auth_token = os.environ.get('Twilio_Token')
    client = Client(account_sid, auth_token)

    message = client.messages.create(
      from_=SENDER,
      body=message_content,
      to=RECIPIENT,
    )

    print(message.sid)

@csrf_exempt
def ajax_view(request):

    if request.META.get('HTTP_X_REQUESTED_WITH') == 'XMLHttpRequest' and request.method == 'GET':

        if "formula" in request.GET:
            formula = request.GET.get('formula')
            relative_radio = request.GET.get('relative_radio')
            absolute_radio = request.GET.get('absolute_radio')
            upper_bound = request.GET.get('upper_bound')
            lower_bound = request.GET.get('lower_bound')
            threshold_check = request.GET.get('threshold_check')
            threshold = request.GET.get('threshold')
            operators = request.GET.get('operators')
            by_pasture = request.GET.get('by_pasture')

            correction_coeff_able = request.GET.get('correction_coeff_able')
            correction_coeff_able = correction_coeff_able.lower() == 'true'

            correction_coeff = request.GET.get('correction_coeff')

            index_image, hist_images, df, geodataframe, centroid, last2_indices, last_2_summary_df, last_2_geodataframe, str_dates = HolderClass.sentinel_request.get_requested_index(formula, relative_radio, absolute_radio, upper_bound, lower_bound, threshold_check, threshold, operators, by_pasture, correction_coeff_able, correction_coeff)

            response_data = {'index_image': index_image, "hist_images": hist_images, "df": df, "geodataframe": geodataframe, "centroid": centroid, "last2_indices": last2_indices, "last_2_summary_df": last_2_summary_df, "last_2_geodataframe": last_2_geodataframe, "str_dates": str_dates}

            return JsonResponse(response_data)

        elif "simulation_data" in request.GET:
            sim_request = json.loads(request.GET['simulation_data'])
            paddocks_grazing_graphs = play_simulation(sim_request)
            return JsonResponse({'paddocks_grazing_graphs': paddocks_grazing_graphs})

        elif "assessDaysLeftDiff" in request.GET:
            reserve = request.GET.get("reserve")
            intake = request.GET.get("intake")
            index = request.GET.get("index")
            pasture_load = json.loads(request.GET.get("pasture_load"))
            averages = json.loads(request.GET.get("averages"))
            RZAs = json.loads(request.GET.get("RZAs"))
            temps = json.loads(request.GET.get("temps"))
            humss = json.loads(request.GET.get("humss"))
            press = json.loads(request.GET.get("press"))

            mask = HolderClass.sentinel_request.masks[int(index)]
            area = (mask.size-mask.sum())*(10**2)/10000

            upper_list = []
            for i in range(3):

                if pasture_load[str(int(index)+1)] == 0:
                    load = sum(pasture_load.values())
                else:
                    load = pasture_load[str(int(index)+1)]

                p_index = averages[i]

                RZA = RZAs[i]; temp = temps[i]; hums = humss[i]; pres = press[i];

                inner_list = [float(p_index), RZA] + [float(temp), float(pres), float(hums)] + [int(load), float(intake), int(reserve)]
                upper_list.append(inner_list)

            model_df = pd.DataFrame(upper_list, columns=MODEL_COLUMNS)

            large_model = HolderClass.sentinel_request.large_model
            norm_model_df = np.array(HolderClass.sentinel_request.norm(model_df))
            another_predictions = large_model.predict(norm_model_df)
            resource_pred = another_predictions[0][:,0]
            days_left_pred = another_predictions[1][:,0]

            return JsonResponse({"resource_pred": resource_pred.tolist(), "days_left_pred": days_left_pred.tolist(), 'area': area})

        elif "takeAction" in request.GET:

            reserve = int(request.GET.get("reserve"))
            intake = int(request.GET.get("intake"))

            temperature = float(request.GET.get("temperature"))
            humidity = float(request.GET.get("humidity"))
            pressure = float(request.GET.get("pressure"))

            pasture_load = [int(x) for x in json.loads(request.GET.get("pasture_load"))]
            days_lefts = [float(x) for x in json.loads(request.GET.get("days_lefts"))]
            resources = [float(x) for x in json.loads(request.GET.get("resources"))]


            data_lables = ["load_of_"+str(p) for p, _ in enumerate(HolderClass.sentinel_request.masks, start=1)] + ["reserve", "daily_intake"] + ["days_left_of_"+str(p) for p, _ in enumerate(HolderClass.sentinel_request.masks, start=1)] + ["temp", "pressure", "humidity"]
            data_draft = [pasture_load+[reserve, intake]+days_lefts+[temperature, pressure, humidity]]

            model_df = pd.DataFrame(data_draft, columns=data_lables)
            model_deep = HolderClass.sentinel_request.control_model

            predictions = model_deep.predict(model_df)

            action = int(predictions[0][7])
            list_of_assesses = predictions[0][:7].tolist()

            return JsonResponse({"action": action, "assesses": list_of_assesses})


        elif "assessDaysLeft" in request.GET:

            reserve = request.GET.get("reserve")
            intake = request.GET.get("intake")
            model = str(request.GET.get("model"))
            pasture_load = json.loads(request.GET.get("pasture_load"))
            resourceValues = json.loads(request.GET.get("resourceValues"))

            RZA = request.GET.get("RZA")
            temperature = request.GET.get("temperature")
            humidity = request.GET.get("humidity")
            pressure = request.GET.get("pressure")

            upper_list = []; area_list = []
            for paddock_id, load in pasture_load.items():

                mask = HolderClass.sentinel_request.masks[int(paddock_id)-1]
                area = (mask.size-mask.sum())*(10**2)/10000
                area_list.append(area)

                if load == 0:
                    load = sum(pasture_load.values())

                p_index = resourceValues[int(paddock_id)-1] #(float(resourceValues[int(paddock_id)-1]) * float(area)) * 1000

                inner_list = [float(p_index), float(RZA)] + [float(temperature), float(pressure), float(humidity)] + [int(load), float(intake), int(reserve)]
                upper_list.append(inner_list)

            model_df = pd.DataFrame(upper_list, columns=MODEL_COLUMNS)


            if model == "ANN":
                large_model = HolderClass.sentinel_request.large_model
                norm_model_df = np.array(HolderClass.sentinel_request.norm(model_df))
                another_predictions = large_model.predict(norm_model_df)
                resource_pred = another_predictions[0][:,0]
                days_left_pred = another_predictions[1][:,0]

            return JsonResponse({"resource_pred": resource_pred.tolist(), "days_left_pred": days_left_pred.tolist(), "area_list": area_list})
        else:
            return JsonResponse({'message': 'Undefined response'})
    else:
        return JsonResponse({'message': 'Ooops response'})


def contains_two_pipe_symbols(input_string):
    return input_string.count(" | ") == 2

def parse_GPS(input_string):
    return [element.strip() for element in input_string.split(" | ")]

def divide_cows(input_string):
    return input_string.split(",")
