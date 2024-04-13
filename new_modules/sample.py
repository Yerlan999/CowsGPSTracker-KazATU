(BLUE+NIR)/(RED+SWIR3)


FireBeetle ESP-32
          Rx | Tx
Serial2 = 16 | 17
Serial1 =  9 | 10
Serial0 =  3 | 1

LoRa:STOP GPS
LoRa:START GPS
LoRa:TIME_UPDATE:1
LoRa:Test



float dummy_coordinates[][2] = {
    {54.214176, 69.511151},
    {54.213195, 69.511303},
    {54.212246, 69.511400},
    {54.211856, 69.512168},
    {54.212011, 69.512357},
    {54.212327, 69.512357},
    {54.212580, 69.512519},
}

latitude = dummy_coordinates[counter][0]; longitude = dummy_coordinates[counter][1];
latitude = dummy_coordinates[cowId][0]; longitude = dummy_coordinates[cowId][1];




Classification report for target column 0:
              precision    recall  f1-score   support

          -1       1.00      1.00      1.00      7407
           0       1.00      1.00      1.00     59207
           1       1.00      1.00      1.00      7474

    accuracy                           1.00     74088
   macro avg       1.00      1.00      1.00     74088
weighted avg       1.00      1.00      1.00     74088

Classification report for target column 1:
              precision    recall  f1-score   support

          -1       1.00      1.00      1.00      7421
           0       1.00      1.00      1.00     22267
           1       1.00      1.00      1.00     44400

    accuracy                           1.00     74088
   macro avg       1.00      1.00      1.00     74088
weighted avg       1.00      1.00      1.00     74088

Classification report for target column 2:
              precision    recall  f1-score   support

          -1       1.00      1.00      1.00      4896
           0       1.00      1.00      1.00     66713
           1       1.00      1.00      1.00      2479

    accuracy                           1.00     74088
   macro avg       1.00      1.00      1.00     74088
weighted avg       1.00      1.00      1.00     74088

Classification report for target column 3:
              precision    recall  f1-score   support

          -1       1.00      1.00      1.00     12307
           0       1.00      1.00      1.00     56828
           1       1.00      1.00      1.00      4953

    accuracy                           1.00     74088
   macro avg       1.00      1.00      1.00     74088
weighted avg       1.00      1.00      1.00     74088

Classification report for target column 4:
              precision    recall  f1-score   support

          -1       1.00      1.00      1.00      9996
           0       1.00      1.00      1.00     54230
           1       1.00      1.00      1.00      9862

    accuracy                           1.00     74088
   macro avg       1.00      1.00      1.00     74088
weighted avg       1.00      1.00      1.00     74088

Classification report for target column 5:
              precision    recall  f1-score   support

          -1       1.00      1.00      1.00     12413
           0       1.00      1.00      1.00     61675

    accuracy                           1.00     74088
   macro avg       1.00      1.00      1.00     74088
weighted avg       1.00      1.00      1.00     74088

Classification report for target column 6:
              precision    recall  f1-score   support

          -1       1.00      1.00      1.00     17232
           0       1.00      1.00      1.00     51936
           1       1.00      1.00      1.00      4920

    accuracy                           1.00     74088
   macro avg       1.00      1.00      1.00     74088
weighted avg       1.00      1.00      1.00     74088

Classification report for target column 7:
              precision    recall  f1-score   support

           0       1.00      1.00      1.00     63794
           1       1.00      1.00      1.00     10294

    accuracy                           1.00     74088
   macro avg       1.00      1.00      1.00     74088
weighted avg       1.00      1.00      1.00     74088




+-----------------+-------------+----------+------------+-----------+
|   Target Column |   Precision |   Recall |   F1-Score |   Support |
+=================+=============+==========+============+===========+
|               0 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               1 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               2 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               3 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               4 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               5 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               6 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               7 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
