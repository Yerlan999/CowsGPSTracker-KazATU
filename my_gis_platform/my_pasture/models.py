from django.db import models

class InputModel(models.Model):
    LEVELS = [
        ("L1C", "Level-1C"),
        ("L2A", "Level-2A"),
    ]

    level = models.CharField(max_length=3, choices=LEVELS)
    start_date = models.DateField()
    end_date = models.DateField()
    recent_date = models.BooleanField()
    kml_file = models.FileField(upload_to='kml_files/')
