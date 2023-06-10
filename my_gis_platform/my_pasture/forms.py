# import the standard Django Forms
# from built-in library
from django import forms
from .models import InputModel
from django.core.exceptions import ValidationError
from datetime import date, timedelta


class DateInput(forms.DateInput):
    input_type = 'date'

# creating a form
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

        if not recent_date and start_date and end_date and start_date >= end_date:
            raise ValidationError('Начальная дата должна быть раньше конечной даты')

        if not recent_date and start_date and end_date and (start_date - end_date < timedelta(days=4)):
            raise ValidationError('Минимальный разрешенный интервал 5 дней')

        if (not recent_date) and (not start_date or not end_date):
            raise ValidationError('Необходимо указать диапазон дат')
