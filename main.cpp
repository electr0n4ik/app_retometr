#include "pugixml.hpp"
#include "gtk/gtk.h"
#include "gdk/gdk.h"
#include <xlnt/xlnt.hpp>
#include <glib.h>
#include <gio/gio.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <thread>
#include <locale>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <cstdlib>
#include <future>
#define make_dir(dir) _wmkdir(dir)
#elif __linux__
#include <unistd.h>
#include <sys/stat.h>
#define make_dir(dir) mkdir(dir, 0777)
#endif


//--------------------------------------//
// Для получения xml файла
GtkBuilder *builder;
//--------------------------------------//
// Создание объектов программы
GtkWidget *window1;
GtkWidget *window2;
GtkWidget *window_termins;
GtkWidget *main_paned1;
GtkWidget *fixed1;
GtkWidget *fixed2;
GtkWidget *scrolled2;
GtkWidget *open_item;
GtkWidget *menu_bar;
GtkWidget *dialog1;
GtkFileChooserAction action;
GtkWidget *labelabout;
GtkWidget *button6;
GtkWidget *export_excel_subitem;

//--------------------------------------//
// таблица в главном окне
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
// столбцы в таблице
GtkCellRenderer *cr1;
GtkCellRenderer *cr2;
GtkCellRenderer *cr3;
GtkCellRenderer *cr4;
GtkCellRenderer *cr5;
// содержимое таблицы
std::string name_object;
std::string start_reg;
std::string schematic_connect;
std::string average_interval;
std::string end_reg;

//--------------------------------------//
// создание таблицы для второй половины главного окна
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
// заголовки столбцов
const gchar *titles[] = {
                "Время", "Uab, B", "Ubc, B", "Uca, B", "Ia, A", "Ib, A", "Ic, A",
                "Ua, B", "Ub, B", "Uc, B", "Pп, Вт", "Pо, Вт", "Pн, Вт", "Qп, Вар",
                "Qо, Вар", "Qн, Вар", "Sп, ВА", "Sо, ВА", "Sн, ВА", "Uп, В", "Uо, В",
                "Uн, В", "Iп, А", "Iо, А", "Iн, А", "Kо", "Kн", "△f, Гц",
                "△Uy, %", "△UyA, %", "△UyB, %", "△UyC, %"
        };

//--------------------------------------//
//ProgressBar для выбора файлов
GtkWidget *bar1;
GtkWidget *button_bar;
GtkWindow *window_bar;
GtkWidget *fixed_bar;
GtkWidget *label_bar;
bool isCanceled = false;
//ProgressBar для экспорта таблицы
GtkWidget *bar2;
GtkWidget *button_bar1;
GtkWindow *window_bar1;
GtkWidget *fixed_bar1;
GtkWidget *label_bar1;
bool isCanceled2 = false;
double progress = 0.0;
gboolean descending = FALSE;

namespace fs = std::filesystem;

// Функция для смены порядка сортировки
void toggle_sort_order(GtkTreeViewColumn *column, gpointer data) {
    descending = !descending;
}

//функция выдает строку с путем к папке с файлом исполнения
std::string getExecutablePath() {
    SetConsoleOutputCP(CP_UTF8);
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

// функция преобразует unix время в "%d.%m.%y %H:%M:%S"
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

void cancelLoading(GtkWidget *widget, gpointer user_data) {
    isCanceled = true;
    gtk_list_store_clear(liststoreResult);
    gtk_list_store_clear(liststore1);
    gtk_widget_destroy(GTK_WIDGET(treeview));
    std::cout << "Отмена";
    gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
    gtk_widget_destroy(GTK_WIDGET(window_bar));
    gtk_widget_set_sensitive(export_excel_subitem, FALSE);
    
}

gboolean on_delete_event_window_bar(GtkWidget *widget, GdkEvent *event, gpointer data) {
    isCanceled = true;
    gtk_list_store_clear(liststoreResult);
    gtk_list_store_clear(liststore1);
    gtk_widget_destroy(GTK_WIDGET(treeview));
    std::cout << "Отмена_крест";
    gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
    gtk_widget_destroy(GTK_WIDGET(window_bar));
    gtk_widget_set_sensitive(export_excel_subitem, FALSE);
    
    return FALSE;
}

void cancelLoading2(GtkWidget *widget, gpointer user_data) {
    isCanceled2 = true;
}

gboolean on_delete_event_window_bar1(GtkWidget *dialog1, GdkEvent *event, gpointer data) {
    isCanceled2 = true;
    return FALSE;
}

void cancelLoading3() {
    gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);

    gtk_widget_destroy(GTK_WIDGET(dialog1));
}

void create_scrollable_table(GtkButton *button, gpointer user_data) {
    // Перед созданием нового GtkScrolledWindow, убедитесь, что treeview не добавлен в другой контейнер
    if (gtk_widget_get_parent(GTK_WIDGET(treeview)) != NULL) {
        return;
    }

    // Удаляем существующие данные из модели данных
    //gtk_list_store_clear(liststoreResult);

    scrolled2 = gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Установка таблицы в контейнер с прокруткой
    gtk_container_add(GTK_CONTAINER(scrolled2), treeview);

    // Добавление GtkScrolledWindow в нижний GtkPaned
    gtk_paned_pack2(GTK_PANED(main_paned1), scrolled2, TRUE, TRUE);

    gtk_widget_show_all(window1);
}

void startLoading() {
    int flag_work = 0;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog1);
    folder_path = gtk_file_chooser_get_filename(chooser);
    for (const auto &entry : std::filesystem::directory_iterator(folder_path)) {
        if (entry.is_directory() || (entry.path().extension() != ".pke")) {
            flag_work = 1;
            break; // Прерываем цикл при обнаружении неправильных файлов или папок
        }
    }
    
    if (flag_work == 0) {
        std::vector<std::string> paramAttrInBlock;
        std::vector<std::string> resultAttrInBlock;
        gtk_list_store_clear(liststoreResult);
        builder = gtk_builder_new_from_file(gladeFilePath.c_str());
        window_bar = GTK_WINDOW(gtk_builder_get_object(builder, "window_bar"));
        gtk_window_set_title(GTK_WINDOW(window_bar), "Загрузка");
        fixed_bar = GTK_WIDGET(gtk_builder_get_object(builder, "fixed_bar"));
        label_bar = GTK_WIDGET(gtk_builder_get_object(builder, "label_bar"));
        button_bar = GTK_WIDGET(gtk_builder_get_object(builder, "button_bar"));
        bar1 = GTK_WIDGET(gtk_builder_get_object(builder, "bar1"));
        int k = 0;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog1);
        folder_path = gtk_file_chooser_get_filename(chooser);
        int numColumns = 32;
        treeview = gtk_tree_view_new();
        renderer = gtk_cell_renderer_text_new();
        std::vector<const char *> values(numColumns);
        GtkTreeViewColumn *columns[numColumns];
        int totalFiles = std::distance(std::filesystem::directory_iterator(folder_path), std::filesystem::directory_iterator{});
        int fileCount = 0;
        progress = 0.0;
        gtk_widget_destroy(dialog1);

        

        for (int i = 0; i < numColumns; ++i) {

            columns[i] = gtk_tree_view_column_new();
            gchar *title = g_strdup_printf("%s", titles[i]);
            gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(columns[i]), title);
            g_free(title);
        
            gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(columns[i]), renderer, TRUE);
            gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(columns[i]), renderer, "text", i);

            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), reinterpret_cast<GtkTreeViewColumn *>(columns[i]));
            
            // Сортировка по клику на заголовок столбца
            gtk_tree_view_column_set_clickable(columns[i], TRUE);
            gtk_tree_view_column_set_sort_column_id(columns[i], i);
            g_signal_connect(columns[i], "clicked", G_CALLBACK(toggle_sort_order), NULL);

        }

        // Устанавливаем модель данных для GtkTreeView
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(liststoreResult));
        
        gtk_widget_show_all(GTK_WIDGET(window_bar));
        g_signal_connect(button_bar, "clicked", G_CALLBACK(cancelLoading), NULL);
        g_signal_connect(window_bar, "delete-event", G_CALLBACK(on_delete_event_window_bar), NULL);
        
        
        for (const auto &entry: std::filesystem::directory_iterator(folder_path)) {
            
            if (isCanceled) {
                
                isCanceled = false;
                progress = 0.0;
                break;
            }
            
            // проход по всем файлам в папке
            // std::string file_path = entry.path().string();
            std::filesystem::path file_path = entry.path().string();
            std::wstring file_path_wide = file_path.wstring();

            // Создает объект doc, который будет представлять загруженный XML-документ.
            pugi::xml_document doc;
            auto doc_load = doc.load_file(file_path.c_str());
            pugi::xml_node root = doc.child("RM3_ПКЭ");

            if (!doc_load || !root.child("Param_Check_PKE") || !root.child("Result_Check_PKE")) {

                fileCount++;
                std::cout << fileCount << " ";
                progress = static_cast<double>(fileCount) / static_cast<double>(totalFiles);
                gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar1), progress);
                gtk_main_iteration();

                if (progress >= 1) {
                    gtk_widget_destroy(GTK_WIDGET(window_bar));
                }

                continue;
            }

            // проход по блокам Param_Check_PKE
            for (pugi::xml_node paramNode = root.child(
                    "Param_Check_PKE"); paramNode; paramNode = paramNode.next_sibling("Param_Check_PKE")) {

                // сохранение переменных <Param_Check_PKE ... /> 
                for (pugi::xml_attribute attr = paramNode.first_attribute(); attr; attr = attr.next_attribute()) {
                    paramAttrInBlock.emplace_back(attr.value());
                }
                break;
            }
            // if (paramAttrInBlock.size() != 6){
            //     continue;
            // }
            

            // проход по блокам Result_Check_PKE
            for (pugi::xml_node resultNode = root.child(
                    "Result_Check_PKE"); resultNode; resultNode = resultNode.next_sibling("Result_Check_PKE")) {

                // все переменные <Result_Check_PKE ... />
                for (pugi::xml_attribute attr = resultNode.first_attribute(); attr; attr = attr.next_attribute()) {
                    resultAttrInBlock.emplace_back(attr.value());
                }
                // if (resultAttrInBlock.size() != 32){
                //     continue;
                // }

                for (int i = 1; i < numColumns + 1; ++i) {
                    values[i] = g_strdup_printf("%s", resultAttrInBlock[i].c_str());
                }

                //TimeTek
                const char *TimeTek = formatted_datetime(values[1]).c_str();

                // Добавляем новую строку
                gtk_list_store_append(liststoreResult, &iterResult);

                gtk_list_store_set(liststoreResult, &iterResult,
                                    0, formatted_datetime(values[1]).c_str(),
                                    1, values[2], 2, values[3], 3, values[4],
                                    4, values[5], 5, values[6], 6, values[7],
                                    7, values[8], 8, values[9], 9, values[10],
                                    10, values[11], 11, values[12], 12, values[13],
                                    13, values[14], 14, values[15], 15, values[16],
                                    16, values[17], 17, values[18], 18, values[19],
                                    19, values[20], 20, values[21], 21, values[22],
                                    22, values[23], 23, values[24], 24, values[25],
                                    25, values[26], 26, values[27], 27, values[28],
                                    28, values[29], 29, values[30], 30, values[31],
                                    31, values[32], -1); 

                resultAttrInBlock.clear();

            }

            if (k == 0) {
                k++;
                // Очищаем список перед добавлением новых строк
                gtk_list_store_clear(liststore1);

                name_object = paramAttrInBlock[2];
                start_reg = paramAttrInBlock[0];
                std::string formattedTimeStart = formatted_datetime(paramAttrInBlock[0]);
                end_reg = paramAttrInBlock[1];
                std::string formattedTimeEnd = formatted_datetime(paramAttrInBlock[1]);

                schematic_connect = paramAttrInBlock[5];
                int number = std::stoi(schematic_connect); // Преобразование строки в число
                // Присваиваем соответствующую строку в зависимости от числа
                switch (number) {
                    case 1:
                        schematic_connect = "1-ф 2-пр";
                        break;
                    case 2:
                        schematic_connect = "3-ф 3-пр";
                        break;
                    case 3:
                        schematic_connect = "3-ф 4-пр";
                        break;
                }
                average_interval = paramAttrInBlock[3] + " мсек";
                GtkTreeIter iter;  // iterators
                gtk_list_store_append(liststore1, &iter);
                gtk_list_store_set(liststore1, &iter, 0, name_object.c_str(), -1); // Название объекта
                gtk_list_store_set(liststore1, &iter, 1, formattedTimeStart.c_str(), -1); // Начало регистрации
                gtk_list_store_set(liststore1, &iter, 2, formattedTimeEnd.c_str(), -1); // Окончание регистрации
                gtk_list_store_set(liststore1, &iter, 3, schematic_connect.c_str(), -1); // Схема соединения
                gtk_list_store_set(liststore1, &iter, 4, average_interval.c_str(), -1); // Интервал усреднения

            }
            
            fileCount++;
            progress = static_cast<double>(fileCount) / static_cast<double>(totalFiles);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar1), progress);
            gtk_main_iteration();

            
            // Если все файлы обработаны, закрываем окно window_bar
            if (progress > 0.99) {
                gtk_widget_destroy(GTK_WIDGET(window_bar));
                
                // Делаем кнопку меню "Экспорт в Excel" активной снова
                gtk_widget_set_sensitive(GTK_WIDGET(export_excel_subitem), TRUE);
            }

            //gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
            // isCanceled = false;
        }
                
    }  else {

        int success_dialog_ok;
        // Выводим сообщение
        GtkWidget *success_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog1),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "Выберите папку с файлами ПКИ.");

        success_dialog_ok = gtk_dialog_run(GTK_DIALOG(success_dialog));

        if (success_dialog_ok) {
            gtk_widget_destroy(success_dialog);
            gtk_widget_destroy(dialog1);
            // Разблокировать окно
            gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
        }

        return;
    
    }
}

void show_folder_dialog() {

    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

    dialog1 = gtk_file_chooser_dialog_new("Выбрать папку",
                                                     GTK_WINDOW(window1),
                                                     action,
                                                     ("Отмена"),
                                                     GTK_RESPONSE_CANCEL,
                                                     ("Выбрать"),
                                                     GTK_RESPONSE_ACCEPT,
                                                     NULL);

    }

void open_file(GtkWidget *open_item, gpointer user_data) {

    // Заблокировать главное окно
    gtk_widget_set_sensitive(GTK_WIDGET(window1), FALSE);

    gint res;   
    show_folder_dialog();
    res = gtk_dialog_run(GTK_DIALOG(dialog1));
    //g_signal_connect(dialog1, "delete-event", G_CALLBACK(on_delete_event_dialog1), NULL);

    // Если пользователь выбрал папку
    if (res == GTK_RESPONSE_ACCEPT) {
        startLoading(); // Вызываем функцию startLoading и получаем окно прогресса
    }
    
    // Если пользователь отменил
    if (res == GTK_RESPONSE_CANCEL) {
        gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
        cancelLoading3();
        gtk_widget_set_sensitive(export_excel_subitem, FALSE);
    }

    if (res == GTK_RESPONSE_DELETE_EVENT){
        cancelLoading3();
    }
    //gtk_widget_destroy(dialog1); // Освобождаем ресурсы диалогового окна
    gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
}

bool copyFile(const std::string& sourcePath, const std::string& destinationPath) {
    try {
        fs::copy(sourcePath, destinationPath, fs::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return false;
    }
}

//--------------------------------------//
//функция экспортирует данные в таблицу excel формата xlsx
void export_to_excel(GtkWidget *export_excel_item, gpointer user_data) {
    SetConsoleOutputCP(CP_UTF8);
    double progress1 = 0.0;
    gtk_widget_set_sensitive(GTK_WIDGET(window1), FALSE);
    builder = gtk_builder_new_from_file(gladeFilePath.c_str());
    window_bar1 = GTK_WINDOW(gtk_builder_get_object(builder, "window_bar1"));
    gtk_window_set_title(GTK_WINDOW(window_bar1), "Загрузка");
    gtk_window_set_resizable(GTK_WINDOW(window_bar1), FALSE);
    fixed_bar1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed_bar1"));
    label_bar1 = GTK_WIDGET(gtk_builder_get_object(builder, "label_bar1"));
    button_bar1 = GTK_WIDGET(gtk_builder_get_object(builder, "button_bar1"));
    bar2 = GTK_WIDGET(gtk_builder_get_object(builder, "bar2"));
    
    GtkTreeModel *model = GTK_TREE_MODEL(liststoreResult);
    GtkTreeIter iter;
    time_t current_time;
    time(&current_time);
    std::string output_filename_str = "retometr_" + std::to_string(current_time) + ".xlsx";
    std::string executable_path = getExecutablePath();
    std::string full_output_path = executable_path + "\\" + output_filename_str;
    xlnt::workbook workbook;
    auto worksheet = workbook.active_sheet();
    int num_rows = gtk_tree_model_iter_n_children(model, NULL);
    int num_columns = gtk_tree_model_get_n_columns(model);
    int max_column_width[num_columns];
    GValue value;
    memset(max_column_width, 0, sizeof(max_column_width));
    std::string start_reg_str = formatted_datetime(start_reg);
    std::string end_reg_str = formatted_datetime(end_reg);
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

    for (int col = 0; col < num_columns - 1; col++) {
        const gchar *column_name = titles[col];
        std::string cell_address = xlnt::cell_reference(col + 1, 4).to_string();
        worksheet.cell(cell_address).value(column_name);
    }

    gtk_widget_show_all(GTK_WIDGET(window_bar1));
    g_signal_connect(button_bar1, "clicked", G_CALLBACK(cancelLoading2), NULL);
    g_signal_connect(window_bar1, "delete-event", G_CALLBACK(on_delete_event_window_bar1), NULL);

    for (int row = 4; row < num_rows + 4; row++) {
        
        if (isCanceled2) {
            workbook.remove_sheet(worksheet);
            gtk_widget_destroy(GTK_WIDGET(window_bar1));
            gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
            isCanceled2 = false;
            break;
        } else {
            for (int col = 0; col < num_columns; col++) {
                gtk_tree_model_iter_nth_child(model, &iter, NULL, row - 4);
                value = G_VALUE_INIT;
                gtk_tree_model_get_value(model, &iter, col, &value);
                if (G_VALUE_HOLDS_STRING(&value)) {
                    const gchar *str_value = g_value_get_string(&value);
                    try {
                        // if (str_value) {
                            worksheet.cell(col + 1, row + 1).value(g_strdup(str_value));
                            int str_lenght = g_utf8_strlen(str_value, -1);
                            if (str_lenght > max_column_width[col]) {
                                max_column_width[col] = str_lenght;
                                worksheet.column_properties(col + 1).width = str_lenght + 3;
                            }
                        // }
                    } catch (const std::exception &e) {
                        worksheet.cell(col + 1, row + 1).value("0");
                    }
                    // std::cout << " str_value " << str_value << " " << row << std::endl;
                }
                
            }
                progress1 = static_cast<double>(row) / static_cast<double>(num_rows);
                gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar2), progress1);

            gtk_main_iteration();
    	}
    }

    if (progress1 >= 1.0) {
        gtk_widget_destroy(GTK_WIDGET(window_bar1));
        isCanceled2 = false;
        std::string sourceFilePath = "C:\\Program Files\\" + output_filename_str;
        workbook.save(output_filename_str);
        #ifdef _WIN32
            std::thread excel_thread([output_filename_str]() { 
                std::string command = "start " + output_filename_str;
                std::system(command.c_str());
            });
            excel_thread.detach();
            GtkWidget *success_dialog = gtk_message_dialog_new(GTK_WINDOW(window1),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_INFO,
                                                       GTK_BUTTONS_OK,
                                                       "Файл открывается...\nФайл успешно экспортирован как:\n%s", output_filename_str.c_str());
            gtk_dialog_run(GTK_DIALOG(success_dialog));
            gtk_widget_destroy(success_dialog);
            
        #elif __linux__
    	    // Открываем файл через xdg-open
    	    if (fork() == 0) {
    		    execlp("xdg-open", "xdg-open", output_filename_str.c_str(), NULL);
    	    }
            gtk_widget_destroy(GTK_WIDGET(window_bar1));

            // Выводим сообщение об успешном экспорте
            GtkWidget *success_dialog = gtk_message_dialog_new(GTK_WINDOW(window1),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_INFO,
                                                       GTK_BUTTONS_OK,
                                                       "Файл успешно экспортирован как:\n%s", full_output_path.c_str());
            gtk_dialog_run(GTK_DIALOG(success_dialog));
            gtk_widget_destroy(success_dialog);
        #endif
        gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
    }
    
}

//--------------------------------------//
//обработчик события для кнопки "ОК", закрывает дочернее окно
void designations(GtkButton *button, gpointer user_data) {
    gtk_widget_destroy(window2);
    g_object_unref(window2);
}

//--------------------------------------//
//обработчик события для кнопки "О программе"
void about_program(GtkWidget *about_program_submenu, gpointer user_data) {
    builder = gtk_builder_new_from_file(gladeFilePath.c_str());
    window2 = GTK_WIDGET(gtk_builder_get_object(builder, "window2"));
    gtk_window_set_title(GTK_WINDOW(window2), "О программе");
    gtk_window_set_resizable(GTK_WINDOW(window2), FALSE);
    fixed2 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed2"));
    labelabout = GTK_WIDGET(gtk_builder_get_object(builder, "labelabout"));
    button6 = GTK_WIDGET(gtk_builder_get_object(builder, "button6"));

    g_signal_connect(button6, "clicked", G_CALLBACK(designations), window2);

    gtk_widget_show_all(window2);
}

//--------------------------------------//
//обработчик события для кнопки "Обозначения"
void about_termins(GtkWidget *about_termins_submenu, gpointer user_data) {
    window_termins = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    gtk_window_set_title(GTK_WINDOW(window_termins), "Обозначения");
    gtk_window_set_default_size(GTK_WINDOW(window_termins), 600, 700);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
    GtkTextView *text_view = GTK_TEXT_VIEW(gtk_text_view_new_with_buffer(buffer));
    gtk_text_view_set_editable(text_view, FALSE);
    gtk_text_view_set_wrap_mode(text_view, GTK_WRAP_WORD_CHAR);
    gtk_text_buffer_set_text(buffer,
        "  Uab - действующее значение напряжения  первой гармоники между фазами A, B\n"
        "  Ubc - действующее значение напряжения  первой гармоники между фазами B, C\n"
        "  Uca - действующее значение напряжения  первой гармоники между фазами C, \n"
        "  Iab - действующее значение тока первой гармоники между фазами A, B\n"
        "  Ibc - действующее значение тока первой гармоники между фазами B, C\n"
        "  Ica - действующее значение тока первой гармоники между фазами C, A\n"
        "  Ia - действующее значение тока первой гармоники фазы A\n"
        "  Ib - действующее значение тока первой гармоники фазы B\n"
        "  Ic - действующее значение тока первой гармоники фазы C\n"
        "  Ua - действующее значение напряжения первой гармоники фазы A\n"
        "  Ub - действующее значение напряжения первой гармоники фазы B\n"
        "  Uc - действующее значение напряжения первой гармоники фазы C\n"
        "  Pп - активная электрическая мощность прямой последовательности\n"
        "  Pо - активная электрическая мощность обратной последовательности\n"
        "  Pн - активная электрическая мощность нулевой последовательности\n"
        "  Qп - реактивная электрическая мощность прямой последовательности\n"
        "  Qо - реактивная электрическая мощность обратной последовательности\n"
        "  Qн - реактивная электрическая мощность нулевой последовательности\n"
        "  Sп - полная электрическая мощность прямой последовательности\n"
        "  Sо - полная электрическая мощность обратной  последовательности\n"
        "  Sн - полная электрическая мощность нулевой последовательности\n"
        "  Uп - значение напряжения переменного тока прямой последовательности\n"
        "  Uо - значение напряжения переменного тока обратной последовательности\n"
        "  Uн - значение напряжения переменного тока нулевой последовательности\n"
        "  Iп - значение силы переменного тока прямой последовательности\n"
        "  Iо - значение силы переменного тока обратной последовательности\n"
        "  Iн - значение силы переменного тока нулевой последовательности\n"
        "  Kо - коэффициент несимметрии напряжения по обратной последовательности\n"
        "  Kн - коэффициент несимметрии напряжения по нулевой последовательности\n"
        "  ∆f, Гц - отклонение частоты\n"
        "  ∆Uy - установившееся отклонение напряжения прямой последовательности\n"
        "  ∆UyAB - установившееся отклонение напряжения между фазами A, B\n"
        "  ∆UyBC - установившееся отклонение напряжения между фазами B, C\n"
        "  ∆UyCA - установившееся отклонение напряжения между фазами C, A\n"
        "  ∆UyA - установившееся отклонение напряжения фазы A\n"
        "  ∆UyB - установившееся отклонение напряжения фазы B\n"
        "  ∆UyC - установившееся отклонение напряжения фазы C\n"
        "  схема проверки 1 = 1-ф 2-пр\n"
        "  схема проверки 2 = 3-ф 3-пр\n"
        "  схема проверки 3 = 3-ф 4-пр", -1);

    GtkWidget *button6 = GTK_WIDGET(gtk_builder_get_object(builder, "button6"));

    g_signal_connect(button6, "clicked", G_CALLBACK(designations), window_termins);

    gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(text_view));
    gtk_container_add(GTK_CONTAINER(window_termins), scrolled_window);

    gtk_widget_show_all(window_termins);
}

void on_main_window_destroy(GtkWidget *widget, gpointer data) {
    // Закрыть остальные окна, например:
    gtk_widget_destroy(GTK_WIDGET(window_bar));
    gtk_widget_destroy(GTK_WIDGET(window_bar1));
    gtk_widget_destroy(GTK_WIDGET(dialog1));
    gtk_widget_destroy(GTK_WIDGET(window2));
    gtk_widget_destroy(GTK_WIDGET(window_termins));

    // Завершить выполнение программы
    gtk_main_quit();
}

//--------------------------------------//
int main(int argc, char *argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    gtk_init(&argc, &argv);
    builder = gtk_builder_new_from_file(gladeFilePath.c_str());
    window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));
    main_paned1 = GTK_WIDGET(gtk_builder_get_object(builder, "main_paned1"));
    menu_bar = gtk_menu_bar_new();
    GtkWidget *pki_item = gtk_menu_item_new_with_label("Файлы ПКЭ");
    GtkWidget *help_item = gtk_menu_item_new_with_label("Помощь");
    GtkWidget *open_item_subitem = gtk_menu_item_new_with_label("Загрузить из файла ПКЭ");
    export_excel_subitem = gtk_menu_item_new_with_label("Экспорт в Excel");
    gtk_widget_set_sensitive(export_excel_subitem, FALSE);
    GtkWidget *about_termins_subitem = gtk_menu_item_new_with_label("Обозначения");
    GtkWidget *about_program_subitem = gtk_menu_item_new_with_label("О программе");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), pki_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);
    GtkWidget *item_submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pki_item), item_submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(item_submenu), open_item_subitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(item_submenu), export_excel_subitem);
    GtkWidget *help_submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_submenu), about_termins_subitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_submenu), about_program_subitem);    
    liststore1 = GTK_LIST_STORE(gtk_builder_get_object(builder, "liststore1"));
    tv1 = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tv1"));
    gtk_widget_set_margin_top(GTK_WIDGET(tv1), 50);
    cx1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx1"));
    cx2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx2"));
    cx3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx3"));
    cx4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx4"));
    cx5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx5"));
    cr1 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr1"));
    cr2 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr2"));
    cr3 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr3"));
    cr4 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr4"));
    cr5 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cr5"));
    gtk_fixed_put(GTK_FIXED(fixed1), menu_bar, 0, 0);
    g_signal_connect(open_item_subitem, "activate", G_CALLBACK(open_file), window1);
    g_signal_connect(export_excel_subitem, "activate", G_CALLBACK(export_to_excel), window1);
    g_signal_connect(about_termins_subitem, "activate", G_CALLBACK(about_termins), window1);
    g_signal_connect(about_program_subitem, "activate", G_CALLBACK(about_program), window1);
    g_signal_connect(tv1, "button-press-event", G_CALLBACK(create_scrollable_table), NULL);
    g_signal_connect(window1, "destroy", G_CALLBACK(on_main_window_destroy), NULL);
    gtk_builder_connect_signals(builder, nullptr);
    gtk_widget_show_all(window1);
    gtk_main();
    g_object_unref(builder);

    return EXIT_SUCCESS;
}
