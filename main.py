class Cell():

    def __init__(self, index):
        self.coverage = ''
        self.plant_height = ''
        self.plant_name = ''
        self.completed = False
        self.plant_volume = ''
        self.cell_button = None
        self.index = index

    def set_coverage(self, coverage):
        self.coverage = coverage

    def set_plant_height(self, plant_height):
        self.plant_height = plant_height

    def set_plant_name(self, plant_name):
        self.plant_name = plant_name
        SubGrid.plant_names.append(self.plant_name)

    def clear(self):

        self.coverage = ''
        self.plant_height = ''
        if (self.plant_name in SubGrid.plant_names):
            SubGrid.plant_names.remove(self.plant_name)
        self.plant_name = ''
        self.completed = False
        self.plant_volume = ''



class SubGrid():
    plant_names = []
    grid = [[0 for _ in range(10)] for _ in range(10)]
    cell_pointer = {"row": 0, "col": 0}

    rows = None
    cols = None
    size = None

    def __init__(self, rows=10, cols=10, size=(10, 10)):
        SubGrid.rows = rows
        SubGrid.cols = cols
        SubGrid.size = size
        SubGrid.create_gird(rows, cols)

    @classmethod
    def create_gird(cls, rows, cols):
        for row in range(rows):
            for col in range(cols):
                cell = Cell((row, col))
                SubGrid.grid[row][col] = cell


    @classmethod
    def calculate_all_cells_yield(cls):
        cell_volumes = dict()
        for row in range(SubGrid.rows):
            for col in range(SubGrid.cols):
                if SubGrid.grid[row][col].completed:
                    SubGrid.grid[row][col].plant_volume = SubGrid.size[0] * SubGrid.size[1] * SubGrid.grid[row][col].coverage/100 * SubGrid.grid[row][col].plant_height
                    if SubGrid.grid[row][col].plant_name in cell_volumes:
                        cell_volumes[SubGrid.grid[row][col].plant_name] += SubGrid.grid[row][col].plant_volume
                    else:
                        cell_volumes[SubGrid.grid[row][col].plant_name] = SubGrid.grid[row][col].plant_volume

        return cell_volumes

    @classmethod
    def calculate_cell_yield(cls):
        row, col = [cell_pointer["row"]], [cell_pointer["col"]]
        grid[row][col].plant_volume = self.size[0] * self.size[1] * grid[ror][col].coverage/100 * grid[row][col].plant_height

    @classmethod
    def plants_name_count(cls):
        count_plants = Counter(SubGrid.plant_names)
        return count_plants

    @classmethod
    def unique_plants_name(cls):
        unique_plants = set(SubGrid.plant_names)
        return unique_plants

    @classmethod
    def update_cell_pointer(cls, row, col):
        SubGrid.cell_pointer["row"] = row
        SubGrid.cell_pointer["col"] = col


class PrimeGrid():
    cell_pointer = {"row": 0, "col": 0}
    grid = [[0 for _ in range(10)] for _ in range(10)]

    def __init__(self, rows=10, cols=10, size=(100, 100)):
        self.rows = rows
        self.cols = cols
        self.size = size



import kivy
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.uix.gridlayout import GridLayout
from kivy.uix.textinput import TextInput
from kivy.uix.boxlayout import BoxLayout
from kivy.core.window import Window
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.uix.popup import Popup
from kivy.config import Config
from functools import partial
from kivy.app import App
import re

Config.set('graphics', 'resizable', True)


class FloatInput(TextInput):

    pat = re.compile('[^0-9]')
    def insert_text(self, substring, from_undo=False):
        pat = self.pat
        if '.' in self.text:
            s = re.sub(pat, '', substring)
        else:
            s = '.'.join(
                re.sub(pat, '', s)
                for s in substring.split('.', 1)
            )
        return super().insert_text(s, from_undo=from_undo)

screen_manager= ScreenManager()

prime_grid_screen = Screen(name='PrimeScreen')
sub_grid_screen = Screen(name='SubScreen')
screen_manager.add_widget(prime_grid_screen)
screen_manager.add_widget(sub_grid_screen)

screen_manager.current = 'SubScreen'

class MyApp(App):

    def calculate_yield(self, plants_dict, unique_plants_name, green_mass_volume, green_mass_yield, *args):
        overall_green_mass_yield = 0
        for plant in unique_plants_name:
            print(plant, plants_dict[plant].text, green_mass_volume[plant])
            overall_green_mass_yield += float(plants_dict[plant].text) * float(green_mass_volume[plant])
        green_mass_yield.text = f"Зеленая масса: {overall_green_mass_yield/1000} кг."

    def exit(self, *args):
        self.get_running_app().stop()

    def fill_plant_name(self, plant_name, plant_name_entry, *args):
        plant_name_entry.text = plant_name

    def calculate(self, *args):
        green_mass_volume = SubGrid.calculate_all_cells_yield()

        popup_main_layout = BoxLayout(orientation='vertical')
        buttons_layout = BoxLayout(orientation='horizontal')
        plant_density_layout = BoxLayout(orientation='vertical')

        header_label = Label(text="Ввод удельной плотности растении (г/см3)")
        plants_dict = dict()
        for plant in SubGrid.unique_plants_name():
            sub_plant_density_layout = BoxLayout(orientation='horizontal')
            plant_label = Label(text=plant)
            plant_entry = FloatInput()
            plants_dict[plant] = plant_entry
            sub_plant_density_layout.add_widget(plant_label)
            sub_plant_density_layout.add_widget(plant_entry)
            plant_density_layout.add_widget(sub_plant_density_layout)

        green_mass_yield = Label(text=f"Зеленая масса: {0} кг.")
        calculate_button = Button(text="Рассчитать")
        close_button = Button(text="Закрыть")
        calculate_button.bind(on_press=partial(self.calculate_yield, plants_dict, SubGrid.unique_plants_name(), green_mass_volume, green_mass_yield))

        popup_main_layout.add_widget(header_label)
        popup_main_layout.add_widget(plant_density_layout)
        popup_main_layout.add_widget(green_mass_yield)
        buttons_layout.add_widget(calculate_button)
        buttons_layout.add_widget(close_button)
        popup_main_layout.add_widget(buttons_layout)

        popup = Popup(title='Расчет зеленой массы', content=popup_main_layout, auto_dismiss=False)
        close_button.bind(on_press=popup.dismiss)
        popup.open()

    def submit_values(self, cell_index, popup, plant_name, coverage, plant_height, *args):
        row, col = SubGrid.cell_pointer["row"], SubGrid.cell_pointer["col"]

        if str(plant_name.text) and str(coverage.text) and str(plant_height.text) and int(coverage.text) <= 100:
            SubGrid.grid[row][col].set_plant_name(str(plant_name.text.strip()))
            SubGrid.grid[row][col].set_coverage(float(coverage.text))
            SubGrid.grid[row][col].set_plant_height(float(plant_height.text))
            SubGrid.grid[row][col].completed = True
            SubGrid.grid[row][col].cell_button.background_color = (0,1,0,1)
            popup.dismiss()

    def clear_values(self, cell_index, popup, *args):
        row, col = SubGrid.cell_pointer["row"], SubGrid.cell_pointer["col"]

        SubGrid.grid[row][col].clear()
        SubGrid.grid[row][col].completed = False
        SubGrid.grid[row][col].cell_button.background_color = (1,1,1,1)
        popup.dismiss()

    def zoom_into(self, cell_index, *args):

        SubGrid.update_cell_pointer(cell_index[0], cell_index[1])

        popup_footer_layout = BoxLayout(orientation='horizontal', size_hint_y=0.2)
        popup_main_layout = BoxLayout(orientation='vertical', spacing=100)
        popup_input_layout = BoxLayout(orientation='vertical', size_hint_y=0.8)
        entered_plants_layout = GridLayout(cols=3, size_hint_y=0.1)

        row, col = SubGrid.cell_pointer["row"], SubGrid.cell_pointer["col"]

        plant_name_label = Label(text='Название растения')
        plant_name = TextInput(text=str(SubGrid.grid[row][col].plant_name))
        coverage_label = Label(text='Проективное покрытие, %')
        coverage = FloatInput(text=str(SubGrid.grid[row][col].coverage))
        plant_height_label = Label(text='Высота травостоя, см')
        plant_height = FloatInput(text=str(SubGrid.grid[row][col].plant_height))

        popup_input_layout.add_widget(plant_name_label)
        popup_input_layout.add_widget(plant_name)
        popup_input_layout.add_widget(coverage_label)
        popup_input_layout.add_widget(coverage)
        popup_input_layout.add_widget(plant_height_label)
        popup_input_layout.add_widget(plant_height)


        if SubGrid.unique_plants_name():
            for plant in SubGrid.unique_plants_name():
                plant_button = Button(text=plant)
                plant_button.bind(on_press=partial(self.fill_plant_name, plant, plant_name))
                entered_plants_layout.add_widget(plant_button)

        close_button = Button(text='Закрыть')
        clear_button = Button(text='Очистить')
        submit_button = Button(text='Ввести')

        popup_footer_layout.add_widget(close_button)
        popup_footer_layout.add_widget(clear_button)
        popup_footer_layout.add_widget(submit_button)

        popup_main_layout.add_widget(popup_input_layout)
        if SubGrid.unique_plants_name():
            popup_main_layout.add_widget(entered_plants_layout)
        popup_main_layout.add_widget(popup_footer_layout)

        popup = Popup(title=f'Ввод данных для ячейки {cell_index[0]+1}:{cell_index[1]+1}', content=popup_main_layout, auto_dismiss=False)

        close_button.bind(on_press=popup.dismiss)
        submit_button.bind(on_press=partial(self.submit_values, cell_index, popup, plant_name, coverage, plant_height))
        clear_button.bind(on_press=partial(self.clear_values, cell_index, popup))

        popup.open()


    def build(self):
        sub_grid = SubGrid()

        header_layout = BoxLayout(orientation='horizontal', size_hint_y=0.1)
        main_layout = BoxLayout(orientation='vertical', spacing=10)
        footer_layout = BoxLayout(orientation='horizontal', size_hint_y=0.1)

        grid_layout = GridLayout(rows=SubGrid.rows, cols=SubGrid.cols)
        for row in range(SubGrid.rows):
            for col in range(SubGrid.cols):
                cell = Button(text=f'{row+1}:{col+1}', background_color = (1, 1, 1, 1))
                cell.bind(on_release=partial(self.zoom_into, (row, col)))
                SubGrid.grid[row][col].cell_button = cell
                grid_layout.add_widget(cell)

        header_button1 = Button(text='Расчет Зеленой Массы на (1м х 1м)')
        header_button1.bind(on_release=self.exit)

        footer_button1 = Button(text='Рассчитать')
        footer_button1.bind(on_release=self.calculate)

        footer_layout.add_widget(footer_button1)

        header_layout.add_widget(header_button1)

        main_layout.add_widget(header_layout)
        main_layout.add_widget(grid_layout)
        main_layout.add_widget(footer_layout)

        return main_layout

MyApp().run()
