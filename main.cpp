#include "pugixml.hpp"
#include "gtk/gtk.h"
#include "gdk/gdk.h"
#include <xlnt/xlnt.hpp>
#include <glib.h>
#include <gio/gio.h> // Для использования GBytes
#include <iostream>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define make_dir(dir) _wmkdir(dir)
#elif __linux__
#include <unistd.h>
#include <sys/stat.h>
#define make_dir(dir) mkdir(dir, 0777)
#endif

// 1. Заменить тестовые кнопки;
// 2. Наполнить раздел "Помощь";
// 3. Закрепить верхнюю половину;
// 4. Добавить полосу загрузки при выполнении операций;
// 5. (Окно предупреждения начала выполенния операции и уведомление об окончании операции);
// 6. Сделать ревью и добавить док-ю;
// 7. Тест на Windows 10.

GtkBuilder *builder;
//--------------------------------------//
// Создание некоторых объектов главного окна
GtkWidget *window1;
GtkWidget *window2;
GtkWidget *main_paned1;
GtkWidget *fixed1;
GtkWidget *fixed2;

GtkWidget *open_item;

//--------------------------------------//
// Получаем тестовый label
GtkWidget *labelabout;

//--------------------------------------//
// Получаем кнопки
GtkWidget *button6;
GtkWidget *export_excel_item;

//--------------------------------------//
// Получаем таблицу в главном окне
// для вывода информации
// о загруженных файлах ПКЭ
GtkWidget *view1;
GtkWidget *view2;
GtkListStore *liststore1;
GtkTreeView *tv1;
GtkTreeViewColumn *cx1;
GtkTreeViewColumn *cx2;
GtkTreeViewColumn *cx3;
GtkTreeViewColumn *cx4;
GtkTreeViewColumn *cx5;

//--------------------------------------//
// Получаем столбцы в таблице
GtkCellRenderer *cr1;
GtkCellRenderer *cr2;
GtkCellRenderer *cr3;
GtkCellRenderer *cr4;
GtkCellRenderer *cr5;

std::string name_object;
std::string start_reg;
std::string schematic_connect;
std::string average_interval;
std::string end_reg;

// Создание таблицы
GtkWidget *treeview;
GtkCellRenderer *renderer;
GtkTreeIter iterResult;

GtkListStore *liststoreResult = gtk_list_store_new(33, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING);

gchar *folder_path;

const gchar *titles[] = {
                "Время", "Uab, B", "Ubc, B", "Uca, B", "Ia, A", "Ib, A", "Ic, A",
                "Ua, B", "Ub, B", "Uc, B", "Pп, Вт", "Pо, Вт", "Pн, Вт", "Qп, Вар",
                "Qо, Вар", "Qн, Вар", "Sп, ВА", "Sо, ВА", "Sн, ВА", "Uп, В", "Uо, В",
                "Uн, В", "Iп, А", "Iо, А", "Iн, А", "Kо", "Kн", "△f, Гц",
                "△Uy, %", "△UyA, %", "△UyB, %", "△UyC, %"
        };

gboolean ASC_sort = FALSE; // Глобальная переменная для отслеживания текущего типа сортировки ASC в порядке возрастания
gboolean DESC_sort = FALSE; // Глобальная переменная для отслеживания текущего типа сортировки DESC в порядке убывания



std::string getExecutablePath() {
    return std::filesystem::current_path().string();
}


// Функция для получения директории из пути
std::string getDirectoryFromPath(const std::string &path) {
    size_t lastSlashPos = path.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        return path.substr(0, lastSlashPos + 1);
    }
    return "";
}

// Получение пути к исполняемому файлу
std::string executablePath = getExecutablePath();
// Получение директории из пути к исполняемому файлу
std::string executableDirectory = getDirectoryFromPath(executablePath);
// Формирование пути к файлу glade.glade (абсолютный или относительный относительно исполняемого файла)
std::string gladeFilePath = executableDirectory + "glade.glade";

// Функция сравнения для сортировки строк (G_TYPE_STRING) в GtkListStore
static gint compare_func(GtkTreeModel *model, GtkTreeIter *iter1, GtkTreeIter *iter2, gpointer user_data) {
    gint column = GPOINTER_TO_INT(user_data);

    gchar *value1, *value2;
    gtk_tree_model_get(model, iter1, column, &value1, -1);
    gtk_tree_model_get(model, iter2, column, &value2, -1);

    // Выполняем сравнение строк с помощью функции g_strcmp0 (без учета регистра)
    gint result = g_strcmp0(value1, value2);

    g_free(value1);
    g_free(value2);

    // Возвращаем результат сравнения
    return result;
}

std::string formatted_datetime(const std::string& unix_time_str) {
    using namespace std::chrono;

    // Преобразование UNIX-времени в системное время (time_point)
    long long unix_time = std::stoll(unix_time_str);
    unix_time /= 1000;
    time_point<system_clock> tp{seconds(unix_time)};

    // Преобразование системного времени в структуру tm
    std::time_t tt = system_clock::to_time_t(tp);
    std::tm local_tm = *std::localtime(&tt);

    // Форматирование времени в строку
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%d.%m.%y %H:%M:%S");
    return oss.str();
}

// Функция обработки сигнала клика по заголовку колонки
void on_column_clicked(GtkTreeViewColumn *column, gpointer user_data) {

    GtkTreeView *treeview = GTK_TREE_VIEW(user_data); // прописать нужный view

    gint current_sort_order = gtk_tree_view_column_get_sort_order(column);

    GtkTreeSortable *sortable = GTK_TREE_SORTABLE(gtk_tree_view_get_model(treeview));

    if (current_sort_order == GTK_SORT_ASCENDING) {
        // Если сортировка по возрастанию, меняем на сортировку по убыванию
        gtk_tree_sortable_set_sort_func(sortable, gtk_tree_view_column_get_sort_column_id(column), compare_func, GINT_TO_POINTER(-1), NULL);
        gtk_tree_sortable_set_sort_column_id(sortable, gtk_tree_view_column_get_sort_column_id(column), GTK_SORT_DESCENDING);
        DESC_sort = TRUE;
    } else if (current_sort_order == GTK_SORT_DESCENDING) {
        // Если сортировка по убыванию, отключаем сортировку
        gtk_tree_sortable_set_sort_func(sortable, gtk_tree_view_column_get_sort_column_id(column), NULL, NULL, NULL);
        gtk_tree_sortable_set_sort_column_id(sortable, -1, GTK_SORT_ASCENDING);
        ASC_sort = FALSE;
        DESC_sort = FALSE;
    } else {
        // Если сортировка отключена, включаем сортировку по возрастанию
        gtk_tree_sortable_set_sort_func(sortable, gtk_tree_view_column_get_sort_column_id(column), compare_func, GINT_TO_POINTER(0), NULL);
        gtk_tree_sortable_set_sort_column_id(sortable, gtk_tree_view_column_get_sort_column_id(column), GTK_SORT_ASCENDING);
        ASC_sort = TRUE;
    }

    // Обновляем вид таблицы
    gtk_tree_view_column_clicked(column);
}

void export_to_excel(GtkWidget *export_excel_item, gpointer user_data) {

    GtkTreeModel *model = GTK_TREE_MODEL(liststoreResult);
    GtkTreeIter iter;

    time_t current_time;
    time(&current_time);
    

    
    std::string output_filename_str = "output_" + std::to_string(current_time) + ".xlsx";

    std::string executable_path = getExecutablePath();

    // Создаем полный путь к файлу Excel рядом с исполняемым файлом
    std::string full_output_path = executable_path + "\\" + output_filename_str;

    // Создаем объект для работы с файлом Excel
    xlnt::workbook workbook;
    xlnt::worksheet worksheet = workbook.active_sheet();

    int num_rows = gtk_tree_model_iter_n_children(model, NULL);
    int num_columns = gtk_tree_model_get_n_columns(model);
    
    // Инициализируем массив max_column_width нулями
    int max_column_width[num_columns];
    memset(max_column_width, 0, sizeof(max_column_width));

    // const char *name_object_exl = name_object.c_str();

    std::string start_reg_str = formatted_datetime(start_reg);
    // const char *start_reg_exl = start_reg_str.c_str();

    // const char *schematic_connect_exl = schematic_connect.c_str();
    // const char *average_interval_exl = average_interval.c_str();

    std::string end_reg_str = formatted_datetime(end_reg);
    // const char *end_reg_exl = end_reg_str.c_str();
    
    worksheet.cell("A1").value("Название объекта:");
    worksheet.cell("B1").value(name_object);
    worksheet.cell("F1").value("Начало регистрации:");
    worksheet.cell("I1").value(start_reg_str);
    worksheet.cell("L1").value("Окончание регистрации:");
    worksheet.cell("O1").value(end_reg_str);

    worksheet.cell("F2").value("Схема соединения:");
    worksheet.cell("I2").value(schematic_connect);
    worksheet.cell("L2").value("Интервал усреднения:");
    worksheet.cell("O2").value(average_interval);

    // Записываем заголовки столбцов в Excel
    for (int col = 0; col < num_columns - 1; col++) {
        const gchar *column_name = titles[col];
        std::string cell_address = xlnt::cell_reference(col + 1, 4).to_string();
        worksheet.cell(cell_address).value(column_name);
    }

    // // Записываем данные из GtkListStore в Excel
    for (int row = 4; row < num_rows + 4; row++) {
        gtk_tree_model_iter_nth_child(model, &iter, NULL, row - 4);

        for (int col = 0; col < num_columns; col++) {
            GValue value = G_VALUE_INIT;
            gtk_tree_model_get_value(model, &iter, col, &value);
            const gchar *str_value = g_value_get_string(&value);

            if (str_value) {
                worksheet.cell(col + 1, row + 1).value(g_strdup(str_value));
                int str_lenght = g_utf8_strlen(str_value, -1);

                if (str_lenght > max_column_width[col]) {
                    max_column_width[col] = str_lenght;
                    // Устанавливаем ширину столбца
                    worksheet.column_properties(col + 1).width = str_lenght + 3;
                }
            }
        }
    }
    
    // Сохраняем файл Excel
    workbook.save(full_output_path);
    
    //Открывает excel, работает на win10(остальные проверить)
    #ifdef _WIN32
    	// Открываем файл в ассоциированном приложении
    	ShellExecuteA(NULL, "open", full_output_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
    #elif __linux__
    	// Открываем файл через xdg-open
    	if (fork() == 0) {
    		execlp("xdg-open", "xdg-open", full_output_path.c_str(), NULL);
    	}
    #endif
}

void create_scrollable_table(GtkButton *button, gpointer user_data) {

    GtkWidget *scrolled2 = gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Установка таблицы в контейнер с прокруткой
    gtk_container_add(GTK_CONTAINER(scrolled2), treeview);

    // Добавление GtkScrolledWindow в нижний GtkPaned
    gtk_paned_pack2(GTK_PANED(main_paned1), scrolled2, TRUE, TRUE);
    
    // Добавляем обработчик клика на заголовок столбцов
    //g_signal_connect(G_OBJECT(treeview), "column-clicked", G_CALLBACK(on_renderer_clicked), NULL);

    gtk_widget_show_all(window1);

}

//--------------------------------------//
//обработчик события для кнопки button6, закрывает дочернее окно
void designations(GtkButton *button, gpointer user_data) {
    gtk_widget_destroy(window2);
    g_object_unref(window2);
}

//--------------------------------------//
//обработчик события для кнопки button3
void about_program(GtkWidget *about_program_submenu, gpointer user_data) {
    builder = gtk_builder_new_from_file(gladeFilePath.c_str());
    window2 = GTK_WIDGET(gtk_builder_get_object(builder, "window2"));
    gtk_window_set_title(GTK_WINDOW(window2), "О программе");

    fixed2 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed2"));
    labelabout = GTK_WIDGET(gtk_builder_get_object(builder, "labelabout"));
    button6 = GTK_WIDGET(gtk_builder_get_object(builder, "button6"));

    g_signal_connect(button6, "clicked", G_CALLBACK(designations), window2);

    gtk_widget_show_all(window2);
}

//--------------------------------------//
//обработчик события для кнопки button3
void about_termins(GtkWidget *about_termins_submenu, gpointer user_data) {
    builder = gtk_builder_new_from_file(gladeFilePath.c_str());
    window2 = GTK_WIDGET(gtk_builder_get_object(builder, "window2"));
    gtk_window_set_title(GTK_WINDOW(window2), "О программе");

    fixed2 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed2"));
    labelabout = GTK_WIDGET(gtk_builder_get_object(builder, "labelabout"));
    button6 = GTK_WIDGET(gtk_builder_get_object(builder, "button6"));

    g_signal_connect(button6, "clicked", G_CALLBACK(designations), window2);

    gtk_widget_show_all(window2);
}

//--------------------------------------//
//обработчик события для кнопки button1
void open_file(GtkWidget *open_item, gpointer user_data) {

    GtkWidget *dialog1;

    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

    gint res;
    int k = 0;
    
    std::vector<std::string> paramAttrInBlock;
    std::vector<std::string> resultAttrInBlock;

    dialog1 = gtk_file_chooser_dialog_new("Выбрать папку",
                                          GTK_WINDOW(user_data),
                                          action,
                                          ("Отмена"),
                                          GTK_RESPONSE_CANCEL,
                                          ("Выбрать"),
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog1));

    // Если пользователь выбрал папку
    if (res == GTK_RESPONSE_ACCEPT) {

        

        // Очищаем список перед добавлением новых строк
        gtk_list_store_clear(liststoreResult);


        

        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog1);
        folder_path = gtk_file_chooser_get_filename(chooser);

        int numColumns = 32; // Количество столбцов

        // Создание таблицы
        //treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(liststoreResult));
        treeview = gtk_tree_view_new(); // Не указываем модель данных здесь
        
        renderer = gtk_cell_renderer_text_new();

        std::vector<const char *> values(numColumns);

        GtkTreeViewColumn *columns[numColumns];
        
        for (int i = 0; i < numColumns; ++i) {

            columns[i] = gtk_tree_view_column_new();
            gchar *title = g_strdup_printf("%s", titles[i]);
            gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(columns[i]), title);
            g_free(title);
	    
            gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(columns[i]), renderer, TRUE);
            gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(columns[i]), renderer, "text", i);

            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), reinterpret_cast<GtkTreeViewColumn *>(columns[i]));
            
            // Установка кликабельности 
            gtk_tree_view_column_set_clickable(columns[i], TRUE);
            g_signal_connect(columns[i], "clicked", G_CALLBACK(on_column_clicked), NULL);
            
        }
        
        // Устанавливаем модель данных для GtkTreeView
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(liststoreResult));


        for (const auto &entry: std::filesystem::directory_iterator(folder_path)) {
            // проход по всем файлам в папке
            std::string file_path = entry.path().string();

            // Создает объект doc, который будет представлять загруженный XML-документ.
            pugi::xml_document doc;

            if (!doc.load_file(file_path.c_str())) {
                std::cerr << "Не удалось загрузить файл: " << file_path << std::endl;
                continue;
                // TODO выводить ошибку один раз и прерывать выполнение функции - "Выбран неверный формат файла!"
            }

            pugi::xml_node root = doc.child("RM3_ПКЭ");

            // проход по блокам Param_Check_PKE
            for (pugi::xml_node paramNode = root.child(
                    "Param_Check_PKE"); paramNode; paramNode = paramNode.next_sibling("Param_Check_PKE")) {
                std::ostringstream oss;
                paramNode.print(oss);

                // сохранение переменных <Param_Check_PKE ... /> в консоль
                for (pugi::xml_attribute attr = paramNode.first_attribute(); attr; attr = attr.next_attribute()) {
                    paramAttrInBlock.emplace_back(attr.value());
                }
                break;
            }

            // проход по блокам Result_Check_PKE
            for (pugi::xml_node resultNode = root.child(
                    "Result_Check_PKE"); resultNode; resultNode = resultNode.next_sibling("Result_Check_PKE")) {
                std::ostringstream oss;
                resultNode.print(oss);

                // все переменные <Result_Check_PKE ... />
                for (pugi::xml_attribute attr = resultNode.first_attribute(); attr; attr = attr.next_attribute()) {
                    resultAttrInBlock.emplace_back(attr.value());
                }
//-----------------------------------------------------------------------------------------------------------------------------------------
                // Добавляем новую строку

                for (int i = 1; i < numColumns + 1; ++i) {
                    values[i] = g_strdup_printf("%s", resultAttrInBlock[i].c_str());
                }

                //TimeTek
                const char *TimeTek = formatted_datetime(values[1]).c_str();

                // Добавляем новую строку
                gtk_list_store_append(liststoreResult, &iterResult);

                gtk_list_store_set(liststoreResult, &iterResult,
                                   0, formatted_datetime(values[1]).c_str(),
                                   1, values[2],
                                   2, values[3],
                                   3, values[4],
                                   4, values[5],
                                   5, values[6],
                                   6, values[7],
                                   7, values[8],
                                   8, values[9],
                                   9, values[10],
                                   10, values[11],
                                   11, values[12],
                                   12, values[13],
                                   13, values[14],
                                   14, values[15],
                                   15, values[16],
                                   16, values[17],
                                   17, values[18],
                                   18, values[19],
                                   19, values[20],
                                   20, values[21],
                                   21, values[22],
                                   22, values[23],
                                   23, values[24],
                                   24, values[25],
                                   25, values[26],
                                   26, values[27],
                                   27, values[28],
                                   28, values[29],
                                   29, values[30],
                                   30, values[31],
                                   31, values[32],
//                                   32, values[33],
                                   -1); // Note: The last argument should be -1 to indicate the end of the list.

//-----------------------------------------------------------------------------------------------------------------------------------------
                resultAttrInBlock.clear();
            }

            if (k == 0) {
                // Очищаем список перед добавлением новых строк
                gtk_list_store_clear(liststore1);

//              Наименование объекта
                name_object = paramAttrInBlock[2];

//              Старт испытания
                start_reg = paramAttrInBlock[0];
                std::string formattedTimeStart = formatted_datetime(paramAttrInBlock[0]);

//              Окончание испытания
                end_reg = paramAttrInBlock[1];
                std::string formattedTimeEnd = formatted_datetime(paramAttrInBlock[1]);

//              Схемы соединения
//              схема проверки 1 = 1-ф 2-пр
//              схема проверки 2 = 3-ф 3-пр
//              схема проверки 3 = 3-ф 4-пр
                schematic_connect = paramAttrInBlock[5];
                int number = std::stoi(schematic_connect); // Преобразование строки в число
                // Присваиваем соответствующую строку в зависимости от числа
                switch (number) {
                    case 1:
                        schematic_connect = "1-ф 2-пр";
                    case 2:
                        schematic_connect = "3-ф 3-пр";
                    case 3:
                        schematic_connect = "3-ф 4-пр";
                }
//              Интервал усреднения
                average_interval = paramAttrInBlock[3] + " мсек";

//                gtk_list_store_clear(liststore1);

                GtkTreeIter iter;  // iterators

                gtk_list_store_append(liststore1, &iter);
                gtk_list_store_set(liststore1, &iter, 0, name_object.c_str(), -1); // Название объекта
                gtk_list_store_set(liststore1, &iter, 1, formattedTimeStart.c_str(), -1); // Начало регистрации
                gtk_list_store_set(liststore1, &iter, 2, formattedTimeEnd.c_str(), -1); // Окончание регистрации
                gtk_list_store_set(liststore1, &iter, 3, schematic_connect.c_str(), -1); // Схема соединения
                gtk_list_store_set(liststore1, &iter, 4, average_interval.c_str(), -1); // Интервал усреднения
            }
            k++;
        }
        g_free(folder_path);
    }
    g_signal_connect(tv1, "row-activated", G_CALLBACK(create_scrollable_table), NULL);
    // Делаем кнопку меню "Экспорт в Excel" активной снова
    gtk_widget_set_sensitive(export_excel_item, TRUE);


    gtk_widget_destroy(dialog1);
}

// Обработчик сигнала для события "destroy" окна window2
void on_window2_destroy(GtkWidget *widget, gpointer user_data) {
    // Закрытие окна window2
    gtk_widget_destroy(GTK_WIDGET(user_data));
    // Выгрузка из памяти builder2 и других связанных объектов
    g_object_unref(builder);
}

//void open_file(GtkMenuItem *menu_item, gpointer user_data) {
//    std::cout << "Hello" << " " << menu_item << std::endl;
//}

//--------------------------------------//
int main(int argc, char *argv[]) {
    
    // Инициализация GTK
    gtk_init(&argc, &argv);

    // Загрузка интерфейса из файла glade.glade
    builder = gtk_builder_new_from_file(gladeFilePath.c_str());

    window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));
    main_paned1 = GTK_WIDGET(gtk_builder_get_object(builder, "main_paned1"));
        
    // Создание header bar
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_widget_show(header_bar);
        
    // Создание собственного виджета для заголовка
    const gchar* str_title = "РЕТОМЕТР-М3";
    GtkWidget *custom_title = gtk_label_new(str_title);
           
    // Добавление header bar к окну и установка свойств
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "РЕТОМЕТР-М3");
    // Установка header bar как заголовка окна
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(window1), header_bar);
        
    // Создание меню-бара
    GtkWidget *menu_bar = gtk_menu_bar_new();
	
    // Создание элементов меню
    GtkWidget *open_item = gtk_menu_item_new_with_label("Загрузить из файла ПКЭ");
    export_excel_item = gtk_menu_item_new_with_label("Экспорт в Excel");
    gtk_widget_set_sensitive(export_excel_item, FALSE);

    GtkWidget *help_item = gtk_menu_item_new_with_label("Помощь");
    // Создание элементов "Помощь" в выпадающем меню
    GtkWidget *about_termins_subitem = gtk_menu_item_new_with_label("Обозначения");
    GtkWidget *about_program_subitem = gtk_menu_item_new_with_label("О программе");
        
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), export_excel_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);
        
    // Создание выпадающего меню в "Помощь"
    GtkWidget *help_submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_submenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(help_submenu), about_termins_subitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_submenu), about_program_subitem);
        
    // Создание модели данных
    liststore1 = GTK_LIST_STORE(gtk_builder_get_object(builder, "liststore1"));
    tv1 = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tv1"));

    cx1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx1"));
    cx2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx2"));
    cx3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx3"));
    cx4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx4"));
    cx5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx5"));

    // Добавление столбцов к таблице
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv1), cx1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv1), cx2);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv1), cx3);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv1), cx4);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv1), cx5);

    cr1 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr1"));
    cr2 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr2"));
    cr3 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr3"));
    cr4 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr4"));
    cr5 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr5"));

    // Добавление меню-бара к header bar
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), menu_bar);
        
    gtk_paned_pack1(GTK_PANED(main_paned1), header_bar, TRUE, TRUE);
    	
    g_signal_connect(G_OBJECT(open_item), "activate", G_CALLBACK(open_file), NULL);
    g_signal_connect(G_OBJECT(export_excel_item), "activate", G_CALLBACK(export_to_excel), NULL);
    g_signal_connect(G_OBJECT(about_termins_subitem), "activate", G_CALLBACK(about_termins), NULL);
    g_signal_connect(G_OBJECT(about_program_subitem), "activate", G_CALLBACK(about_program), NULL);
        
    g_signal_connect(window1, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window2, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        
    gtk_builder_connect_signals(builder, nullptr);
    gtk_widget_show_all(window1);
    gtk_main();
    g_object_unref(builder);

    return EXIT_SUCCESS;
}
