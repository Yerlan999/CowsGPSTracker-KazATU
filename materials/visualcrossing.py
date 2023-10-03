Sample_URL = "https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/54.214106,69.519512/2022-05-01/2022-08-31?key="
BaseURL = 'https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/'

ApiKey=''
#UnitGroup sets the units of the output - us or metric
UnitGroup='metric'

#Location for the weather data
Location='54.214106,69.519512'

#Optional start and end dates
#If nothing is specified, the forecast is retrieved.
#If start date only is specified, a single historical or forecast day will be retrieved
#If both start and and end date are specified, a date range will be retrieved
StartDate = '2023-05-01'
EndDate='2023-08-31'

#JSON or CSV
#JSON format supports daily, hourly, current conditions, weather alerts and events in a single JSON package
#CSV format requires an 'include' parameter below to indicate which table section is required
ContentType="json"

#include sections
#values include days,hours,current,alerts
Include="days"


#basic query including location
ApiQuery=BaseURL + Location

#append the start and end date if present
if (len(StartDate)):
    ApiQuery+="/"+StartDate
    if (len(EndDate)):
        ApiQuery+="/"+EndDate

#Url is completed. Now add query parameters (could be passed as GET or POST)
ApiQuery+="?"

#append each parameter as necessary
if (len(UnitGroup)):
    ApiQuery+="&unitGroup="+UnitGroup

if (len(ContentType)):
    ApiQuery+="&contentType="+ContentType

if (len(Include)):
    ApiQuery+="&include="+Include

ApiQuery+="&key="+ApiKey



print(' - Running query URL: ', ApiQuery)
print()

try:
    # Convert from bytes to text
    resp_text = urllib.request.urlopen(ApiQuery).read().decode('UTF-8')
    # Use loads to decode from text
    json_obj = json.loads(resp_text)
#     print(json_obj)
#     print("JSON Object: ")

except urllib.error.HTTPError  as e:
    ErrorInfo= e.read().decode()
    print('Error code: ', e.code, ErrorInfo)
    sys.exit()
except  urllib.error.URLError as e:
    ErrorInfo= e.read().decode()
    print('Error code: ', e.code,ErrorInfo)
    sys.exit()



df = pd.DataFrame(json_obj["days"])

excel_file_path = f'Weather for from {StartDate} to {EndDate}.xlsx'
df.to_excel(excel_file_path, index=False)
