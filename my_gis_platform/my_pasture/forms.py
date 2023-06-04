# import the standard Django Forms
# from built-in library
from django import forms
from .models import InputModel
from django.core.exceptions import ValidationError


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
        }

    def clean(self):
        cleaned_data = super().clean()
        start_date = cleaned_data.get('start_date')
        end_date = cleaned_data.get('end_date')
        recent_date = cleaned_data.get('recent_date')

        if not recent_date and start_date and end_date and start_date >= end_date:
            raise ValidationError('Начальная дата должна быть раньше конечной даты')

        return cleaned_data

