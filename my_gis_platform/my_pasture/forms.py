from django import forms
from .models import InputModel
from django.core.exceptions import ValidationError
from datetime import date, timedelta
import re


def is_english_letters_only(filename):
    pattern = r'^[a-zA-Z.]+$'
    return re.match(pattern, filename) is not None

class DateInput(forms.DateInput):
    input_type = 'date'


class InputForm(forms.ModelForm):

    class Meta:
        model = InputModel
        fields = "__all__"
        labels = {
            "level":("Уровень спутника"),
            "start_date":("Начальная дата"),
            "end_date":("Конечная дата"),
            "recent_date":("Последняя доступная дата"),
            "kml_file":("KML файл"),
        }
        help_texts = {
            "kml_file":("KML файл с координатами пастбища"),
        }
        widgets = {
            "start_date": DateInput,
            "end_date": DateInput,
            'kml_file': forms.ClearableFileInput(attrs={'accept': '.kml, .shp'}),
        }

    def clean(self):
        cleaned_data = super().clean()
        start_date = cleaned_data.get('start_date')
        end_date = cleaned_data.get('end_date')
        recent_date = cleaned_data.get('recent_date')
        kml_file = cleaned_data.get('kml_file')

        if not is_english_letters_only(kml_file.name):
            raise ValidationError("Название файла должно быть на английском")

        if not recent_date and start_date and end_date and start_date >= end_date:
            raise ValidationError('Начальная дата должна быть раньше конечной даты')

        if not recent_date and start_date and end_date and (end_date - start_date < timedelta(days=8)):
            raise ValidationError('Минимальный разрешенный интервал 9 дней')

        if (not recent_date) and (not start_date or not end_date):
            raise ValidationError('Необходимо указать диапазон дат')
