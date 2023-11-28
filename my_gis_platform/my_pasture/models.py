from django.db import models
from django.core.validators import FileExtensionValidator

import os
from django.db.models.signals import post_save
from django.dispatch import receiver
from django.conf import settings
from django.core.files.storage import FileSystemStorage
from django.core.files.storage import default_storage
from django.core.files.storage import get_storage_class


@receiver(post_save)
def clear_media_folder(sender, instance, **kwargs):
    folder_name = 'kml_files'
    file_field_name = 'kml_file'

    media_path = os.path.join(settings.MEDIA_ROOT, folder_name)

    if hasattr(instance, file_field_name) and instance.kml_file:
        try:
            storage_class = get_storage_class()
            storage = storage_class()
            storage_location = storage.location

            media_storage = FileSystemStorage(location=media_path)
            for file_name in media_storage.listdir('')[1]:
                directory, existing_file_name = os.path.split(str(getattr(instance, file_field_name)))
                if file_name != existing_file_name:
                    file_path = os.path.join(media_path, file_name)
                    os.remove(file_path)
        except Exception as e:
            print(f"Error deleting files in {media_path}: {e}")

post_save.connect(clear_media_folder)


class InputModel(models.Model):
    LEVELS = [
        ("L1C", "Level-1C"),
        ("L2A", "Level-2A"),
    ]

    def save(self, *args, **kwargs):
        self.pk = 1
        super().save(*args, **kwargs)

    def delete(self, *args, **kwargs):
        pass

    level = models.CharField(max_length=3, choices=LEVELS)
    start_date = models.DateField(null=True, blank=True)
    end_date = models.DateField(null=True, blank=True)
    recent_date = models.BooleanField()
    kml_file = models.FileField(upload_to='kml_files/', validators=[FileExtensionValidator(allowed_extensions=['kml'])])
