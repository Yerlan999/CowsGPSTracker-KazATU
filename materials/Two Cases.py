        self.ULTRA_BLUE = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B01"]])))

        self.BLUE = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B02"]])))
        self.GREEN = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B03"]])))
        self.RED = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B04"]])))

        self.RED_EDGE1 = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B05"]])))
        self.RED_EDGE2 = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B06"]])))
        self.RED_EDGE3 = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B07"]])))

        self.NIR = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B08"]])))
        self.N_NIR = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B8A"]])))
        self.WV = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B09"]])))
        if "B10" in self.bands_dict:
            self.SWIR_C = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B10"]])))
        self.SWIR2 = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B11"]])))
        self.SWIR3 = self.pasture(normalize(brighten(self.all_bands_data[self.image_date][:, :, self.bands_dict["B12"]])))

        self.P_ULTRA_BLUE = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B01"]])))
        self.P_BLUE = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B02"]])))
        self.P_GREEN = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B03"]])))
        self.P_RED = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B04"]])))
        self.P_RED_EDGE1 = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B05"]])))
        self.P_RED_EDGE2 = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B06"]])))
        self.P_RED_EDGE3 = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B07"]])))
        self.P_NIR = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B08"]])))
        self.P_N_NIR = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B8A"]])))
        self.P_WV = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B09"]])))
        if "B10" in self.bands_dict:
            self.P_SWIR_C = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B10"]])))
        self.P_SWIR2 = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B11"]])))
        self.P_SWIR3 = normalize(brighten(self.pasture(self.all_bands_data[self.image_date][:, :, self.bands_dict["B12"]])))



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
