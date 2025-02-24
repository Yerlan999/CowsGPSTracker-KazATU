# Generated by Django 4.1 on 2023-06-04 12:23

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ("my_pasture", "0002_rename_member_user"),
    ]

    operations = [
        migrations.CreateModel(
            name="InputModel",
            fields=[
                (
                    "id",
                    models.BigAutoField(
                        auto_created=True,
                        primary_key=True,
                        serialize=False,
                        verbose_name="ID",
                    ),
                ),
                (
                    "level",
                    models.CharField(
                        choices=[("L1C", "Level-1C"), ("L2A", "Level-2A")], max_length=3
                    ),
                ),
                ("start_date", models.DateField()),
                ("end_date", models.DateField()),
                ("recent_date", models.BooleanField()),
                ("kml_file", models.FileField(upload_to="kml_files/")),
            ],
        ),
        migrations.DeleteModel(
            name="User",
        ),
    ]
