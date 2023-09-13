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
#include <limits.h>
#include <cstdlib>
#include <sstream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <stdbool.h>


#include <cerrno>
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

GtkWidget *open_item;
GtkWidget *menu_bar;
GtkWidget *dialog1;
GtkFileChooserAction action;
GtkWidget *labelabout;
GtkWidget *button6;
GtkWidget *export_excel_subitem;
GtkWidget *drop_widget;
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
GtkListStore *liststoreResult;
GtkListStore *liststoreResult1;
GtkListStore *liststoreResult2;
GtkListStore *liststoreResult3;
// static const gchar *titles[33];

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
GtkWidget *scrolled2;
GtkScrolledWindow* scrolled2_window = NULL;

std::filesystem::path folder_path;
// std::string folder_path_decode;
bool dnd_flag = true;
// bool flag_work = false;
bool columns_created = false;
const gchar** titles;
static const gchar *titles1[] = {"Время", "Ua, B", "Ia, A", "Pa, Вт", "Qa, Вар", "Sa, ВА", "△f, Гц", "△Uy, %"};
static const gchar *titles2[] = {
                    "Время", "Uab, B", "Ubc, B", "Uca, B", 
                    "Iab, A", "Ibc, A", "Ica, A", "Ia, A", 
                    "Ib, A", "Ic, A", "Pп, Вт", "Pо, Вт", 
                    "Qп, Вар", "Qо, Вар", "Sп, ВА", "Sо, ВА", 
                    "Uп, В", "Uо, В", "Iп, А", "Iо, А", 
                    "Kо", "△f, Гц", "△Uy, %", "△UyAB, %", 
                    "△UyBC, %", "△UyCA, %"};
static const gchar *titles3[] = {
                    "Время", "Uab, B", "Ubc, B", "Uca, B", "Ia, A", "Ib, A", "Ic, A", "Ua, B", "Ub, B", "Uc, B", 
                    "Po, Bт", "Pп, Вт", "Pн, Вт", "Qо, Вар", "Qп, Вар", "Qн, Вар", "Sо, ВА", "Sп, ВА", "Sн, ВА", 
                    "Uо, В", "Uп, В", "Uн, В", "Iо, А", "Iп, А", "Iн, А", "Kо", "Kн", "△f, Гц", "△Uy, %", 
                    "△UyA, %", "△UyB, %", "△UyC, %"};
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
float progress = 0.0;
gboolean descending = FALSE;
int numcolumns_old = 0;

// Функция для смены порядка сортировки
void toggle_sort_order(GtkTreeViewColumn *column, gpointer data) {
    descending = !descending;
}

//функция выдает строку с путем к папке с файлом исполнения
std::string getExecutablePath() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        return std::filesystem::current_path().string();
    #elif __linux__
    	char result[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
		if (count != -1) {
		    result[count] = '\0';
		    return std::string(result);
		} else {
		    return "";
		}
    #endif
    
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
    long long unix_time = 0;
    //long long unix_time = std::stoll(unix_time_str);
    
	if (!unix_time_str.empty() && std::all_of(unix_time_str.begin(), unix_time_str.end(), ::isdigit)) {
		unix_time = std::stoll(unix_time_str);
	}
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
    gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
    gtk_widget_destroy(GTK_WIDGET(window_bar));
    if (!columns_created)
    {
    gtk_widget_set_sensitive(export_excel_subitem, FALSE);
    }
    
}

gboolean on_delete_event_window_bar(GtkWidget *widget, GdkEvent *event, gpointer data) {
    isCanceled = true;
    gtk_list_store_clear(liststoreResult);
    gtk_list_store_clear(liststore1);
    gtk_widget_destroy(GTK_WIDGET(treeview));
    gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
    gtk_widget_destroy(GTK_WIDGET(window_bar));
    if (!columns_created)
    {
    gtk_widget_set_sensitive(export_excel_subitem, FALSE);
    }
    
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

void create_scrollable_table(GtkButton *button, gpointer user_data) 
{
    if (gtk_widget_get_parent(GTK_WIDGET(treeview)) != NULL) 
    {
        return;
    }
    scrolled2 = gtk_scrolled_window_new(NULL, NULL);
    scrolled2_window = GTK_SCROLLED_WINDOW(scrolled2);  
    columns_created = true;
    gtk_scrolled_window_set_policy(scrolled2_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled2_window), GTK_WIDGET(treeview));
    gtk_paned_pack2(GTK_PANED(main_paned1), GTK_WIDGET(scrolled2_window), TRUE, TRUE);
    

    gtk_widget_show_all(GTK_WIDGET(window1));
}

std::string convert_to_utf8(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(str);
}

void startLoading() {

    // gtk_widget_set_sensitive(GTK_WIDGET(export_excel_subitem), TRUE);
    
    if (columns_created){
        gtk_widget_hide(GTK_WIDGET(scrolled2_window));
        gtk_widget_hide(GTK_WIDGET(scrolled2));
        // gtk_widget_hide(GTK_WIDGET(treeview));
        gtk_widget_destroy(GTK_WIDGET(treeview));
        // Получите модель данных
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tv1));

        // Очистите модель данных
        if (GTK_IS_LIST_STORE(model)) {
            gtk_list_store_clear(GTK_LIST_STORE(model));
        } else if (GTK_IS_TREE_STORE(model)) {
            gtk_tree_store_clear(GTK_TREE_STORE(model));
        }

        // Обновите вид
        gtk_tree_view_set_model(GTK_TREE_VIEW(tv1), NULL);
        gtk_tree_view_set_model(GTK_TREE_VIEW(tv1), model);
             
        columns_created = false;
        gtk_container_remove(GTK_CONTAINER(main_paned1), GTK_WIDGET(scrolled2_window));

    }
    
    float totalFiles = 0;
    int number;
    float fileCount = 0.0;
    float progress = 0.0;
    int k = 0;
    int numColumns = 0;
    name_object = "";
    start_reg = "";
    schematic_connect = "";
    average_interval = "";
    end_reg = "";

    if (dnd_flag)
    {
        
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog1);

		#ifdef _WIN32
		    // folder_path = gtk_file_chooser_get_filename(chooser);
            std::string path = gtk_file_chooser_get_filename(chooser);

            int len = path.length();
            for (int i = 0; i < len; i++) {
                if (path[i] == '\\') {
                    path[i] = '/';
                }
            }

            folder_path = path;
		#elif __linux__
			const gchar *folder_path_decode = gtk_file_chooser_get_filename(chooser);
			folder_path = std::filesystem::u8path(folder_path_decode);
		#endif
		gtk_widget_destroy(dialog1);
    } 
    std::cout << folder_path << std::endl;
	if (std::filesystem::is_directory(folder_path)) 
    {
		for (const auto &entry : std::filesystem::directory_iterator(folder_path)) 
        {
		    if (entry.is_directory()) 
            {
                std::cout << "1return";
                dnd_flag = true;
                return;
		    } else 
            {
				if (entry.path().extension() == ".pke")
                { //|| entry.path().extension() == ".xml") {
					continue; 
				} else 
                {
                    int success_dialog_ok;
                    // Выводим сообщение
                    GtkWidget *success_dialog = gtk_message_dialog_new(GTK_WINDOW(window1),
                                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                                GTK_MESSAGE_INFO,
                                                                GTK_BUTTONS_OK,
                                                                "Папка содежит неверные файлы!");
                    gtk_window_set_modal(GTK_WINDOW(success_dialog), TRUE);

                    success_dialog_ok = gtk_dialog_run(GTK_DIALOG(success_dialog));

                    if (success_dialog_ok) 
                    {
                        gtk_widget_destroy(success_dialog);
                    }
                    std::cout << "2return";
                    dnd_flag = true;
                    return;
				}
			}
	    }
    } else 
    {
        std::cout << "3return";
        if(dnd_flag) 
        {
            gtk_widget_destroy(dialog1);
        }
        totalFiles = 0;
        fileCount = 0;
        number = 0;
        folder_path = "";
        dnd_flag = true;
        return;
    }
    
	totalFiles = std::distance(std::filesystem::directory_iterator(folder_path), std::filesystem::directory_iterator{});

    if (totalFiles == 0){
        return;
    }

    std::vector<std::string> paramAttrInBlock;
    std::vector<std::string> resultAttrInBlock;

    builder = gtk_builder_new_from_file(gladeFilePath.c_str());
    
    window_bar = GTK_WINDOW(gtk_builder_get_object(builder, "window_bar"));
    gtk_window_set_title(GTK_WINDOW(window_bar), "Загрузка");
    fixed_bar = GTK_WIDGET(gtk_builder_get_object(builder, "fixed_bar"));
    label_bar = GTK_WIDGET(gtk_builder_get_object(builder, "label_bar"));
    button_bar = GTK_WIDGET(gtk_builder_get_object(builder, "button_bar"));
    bar1 = GTK_WIDGET(gtk_builder_get_object(builder, "bar1"));

    // treeview = gtk_tree_view_new();

    renderer = gtk_cell_renderer_text_new();


    std::cout << " Отладка " << std::endl;
    gtk_widget_show_all(GTK_WIDGET(window_bar));
    g_signal_connect(button_bar, "clicked", G_CALLBACK(cancelLoading), NULL);
    g_signal_connect(window_bar, "delete-event", G_CALLBACK(on_delete_event_window_bar), NULL);
    std::cout << folder_path << std::endl;
    for (const auto &entry: std::filesystem::directory_iterator(folder_path)) {
        
        if (isCanceled) {
            isCanceled = false;
            progress = 0.0;
            break;
        }
        std::filesystem::path file_path = entry.path().string();
        pugi::xml_document doc;

         if (!doc.load_file(entry.path().c_str())) {
             fileCount = fileCount + 1.0;

             progress = fileCount / totalFiles;
             gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar1), progress);
             gtk_main_iteration();

             if (progress >= 1) {
                 gtk_widget_destroy(GTK_WIDGET(window_bar));
             }
             
             continue;
             
         }
		
        pugi::xml_node root = doc.child("RM3_ПКЭ");

        if (k == 0) {
				k++;
				for (pugi::xml_node paramNode = root.child("Param_Check_PKE"); paramNode; paramNode = paramNode.next_sibling("Param_Check_PKE")) {
				
					// сохранение переменных <Param_Check_PKE ... /> в консоль
					for (pugi::xml_attribute attr = paramNode.first_attribute(); attr; attr = attr.next_attribute()) {
						paramAttrInBlock.emplace_back(attr.value());
					}
					break;
				}

				gtk_list_store_clear(liststore1);

				name_object = paramAttrInBlock[2].c_str();

				start_reg = paramAttrInBlock[0].c_str();
				std::string formattedTimeStart = formatted_datetime(paramAttrInBlock[0]);

				end_reg = paramAttrInBlock[1].c_str();
				std::string formattedTimeEnd = formatted_datetime(paramAttrInBlock[1]);

				schematic_connect = paramAttrInBlock[5];
				number = std::stoi(schematic_connect); 

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
                // average_interval_int = std::stoi(paramAttrInBlock[3]) / 1000; 
				average_interval = paramAttrInBlock[3] + " мсек";
				GtkTreeIter iter;  
				gtk_list_store_append(liststore1, &iter);
				gtk_list_store_set(liststore1, &iter, 0, name_object.c_str(), -1); // Название объекта
				gtk_list_store_set(liststore1, &iter, 1, formattedTimeStart.c_str(), -1); // Начало регистрации
				gtk_list_store_set(liststore1, &iter, 2, formattedTimeEnd.c_str(), -1); // Окончание регистрации
				gtk_list_store_set(liststore1, &iter, 3, schematic_connect.c_str(), -1); // Схема соединения
				gtk_list_store_set(liststore1, &iter, 4, average_interval.c_str(), -1); // Интервал усреднения

                gtk_list_store_clear(liststoreResult);
				if (number == 1) {
					numColumns = 8;
                    // columns[8];

					liststoreResult1 = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING
												         );
                    liststoreResult = liststoreResult1;
                    
					titles = titles1;
				} else if (number == 2) {
					numColumns = 26;
                    // columns[26];


					liststoreResult2 = gtk_list_store_new(26, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING);
                    liststoreResult = liststoreResult2;
                    
					titles = titles2;
				} else if (number == 3) {
					numColumns = 32;
                    // columns[32];

					liststoreResult3 = gtk_list_store_new(32, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
												         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
					liststoreResult = liststoreResult3;
                    
                    titles = titles3;

				}
		}

        if (k == 1)
        {
            k++;
            GtkTreeViewColumn *columns[numColumns];
            
            if (columns_created) 
            {

                for (int i = 0; i < numColumns; ++i) 
                {
                    columns[i] = NULL;
                }

                if (scrolled2_window != NULL) 
                {
                    gtk_container_remove(GTK_CONTAINER(main_paned1), GTK_WIDGET(scrolled2_window));
                    scrolled2_window = NULL;
                    treeview = NULL; 
                }
                std::cout << " Очистил ";
                
                columns_created = true;
            }
            treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(liststoreResult));
            
            for (int i = 0; i < numColumns; ++i) 
            {
                columns[i] = gtk_tree_view_column_new();
                gchar *title = g_strdup_printf("%s", titles[i]);
                gtk_tree_view_column_set_title(columns[i], title);
                g_free(title);
                gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(columns[i]), renderer, TRUE);
                gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(columns[i]), renderer, "text", i);
                gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), reinterpret_cast<GtkTreeViewColumn *>(columns[i]));
                gtk_tree_view_column_set_clickable(columns[i], TRUE);
                gtk_tree_view_column_set_sort_column_id(columns[i], i);
                g_signal_connect(columns[i], "clicked", G_CALLBACK(toggle_sort_order), NULL);
            }
            
        }
        
		if (k == 2)
        {
           
			for (pugi::xml_node resultNode = root.child("Result_Check_PKE"); resultNode; resultNode = resultNode.next_sibling("Result_Check_PKE")) {
				
				gchar* values[numColumns] = { nullptr };
                // gchar* values[numColumns] = { "Err" };
				
				for (pugi::xml_attribute attr = resultNode.first_attribute(); attr; attr = attr.next_attribute()) {
					resultAttrInBlock.emplace_back(attr.value());
					//std::cout << attr.value();
				}
				
                if(resultAttrInBlock != numColumns){
                    cancelLoading3();
                }

				for (int i = 0; i < numColumns + 1; ++i) {
                    values[i] = g_strdup_printf("%s", resultAttrInBlock[i].c_str());
                }

				if (number == 1) {
					gtk_list_store_append(liststoreResult, &iterResult);
					
					gtk_list_store_set(liststoreResult, &iterResult,
										 0, formatted_datetime(values[1]).c_str(),
										 1, values[2], 2, values[3], 3, values[4],
										 4, values[5], 5, values[6], 6, values[7],
										 7, values[8], -1); 
				} else if (number == 2) {
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
									 25, values[26], -1); 
				} else if (number == 3) {
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
				}
                
				resultAttrInBlock.clear();
			}
        }
         
        fileCount = fileCount + 1.0;
        progress = fileCount / totalFiles;
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar1), progress);
        gtk_main_iteration();


        if (progress > 0.99) {
            gtk_widget_destroy(GTK_WIDGET(window_bar));

            gtk_widget_set_sensitive(GTK_WIDGET(export_excel_subitem), TRUE);
        }
        isCanceled = false;
    }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(liststoreResult));
    k = 0;
    totalFiles = 0;
    fileCount = 0;
    number = 0;
    folder_path = "";
    dnd_flag = true;
    //gtk_widget_show_all(GTK_WIDGET(window1));

}

void open_file(GtkWidget *open_item, gpointer user_data) {

    // gtk_widget_set_sensitive(GTK_WIDGET(window1), FALSE);

    gint res;   
    // show_folder_dialog();
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

    dialog1 = gtk_file_chooser_dialog_new("Выбрать папку",
                                                     GTK_WINDOW(window1),
                                                     action,
                                                     ("Отмена"),
                                                     GTK_RESPONSE_CANCEL,
                                                     ("Выбрать"),
                                                     GTK_RESPONSE_ACCEPT,
                                                     NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog1), FALSE); // Отключить выбор нескольких элементов
    
    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog1), FALSE); // Отключить локальность
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog1), FALSE); // Отключить выбор нескольких элементов
    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog1), FALSE); // Отключить отображение скрытых файлов и папок
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog1), FALSE); // Отключить подтверждение при перезаписи
    gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(dialog1), FALSE); // Отключить создание папок

    GtkFileFilter *folder_filter = gtk_file_filter_new();
    gtk_file_filter_add_mime_type(folder_filter, "inode/directory"); // Миме-тип для папок
    gtk_file_filter_set_name(folder_filter, "Только папки");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog1), folder_filter);
                                       

    res = gtk_dialog_run(GTK_DIALOG(dialog1));
    std::cout << res << std::endl;

    if (res == GTK_RESPONSE_ACCEPT) 
    {
    startLoading();
    }

    if (res == GTK_RESPONSE_CANCEL) {
        // gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
        cancelLoading3();

    }

    if (res == GTK_RESPONSE_DELETE_EVENT){
        cancelLoading3();
    }
    res = 0;
    // gtk_widget_destroy(dialog1); // Освобождаем ресурсы диалогового окна
    gtk_widget_set_sensitive(GTK_WIDGET(window1), TRUE);
}

void export_to_excel(GtkWidget *export_excel_item, gpointer user_data) {
    #ifdef _WIN32
		SetConsoleOutputCP(CP_UTF8);
    #endif
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

    for (int col = 0; col < num_columns; col++) {
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

void designations(GtkButton *button, gpointer user_data) {
    gtk_widget_destroy(window2);
    g_object_unref(window2);
}

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
    // gtk_widget_destroy(GTK_WIDGET(window_bar));
    // gtk_widget_destroy(GTK_WIDGET(window_bar1));
    // gtk_widget_destroy(GTK_WIDGET(dialog1));
    // gtk_widget_destroy(GTK_WIDGET(window2));
    // gtk_widget_destroy(GTK_WIDGET(window_termins));
    if (window_bar != nullptr) {
        gtk_widget_destroy(GTK_WIDGET(window_bar));
        window_bar = nullptr;
    }
    if (window_bar1 != nullptr) {
        gtk_widget_destroy(GTK_WIDGET(window_bar1));
        window_bar1 = nullptr;
    }
    if (dialog1 != nullptr) {
        gtk_widget_destroy(GTK_WIDGET(dialog1));
        dialog1 = nullptr;
    }
    if (window2 != nullptr) {
        gtk_widget_destroy(GTK_WIDGET(window2));
        window2 = nullptr;
    }
    if (window_termins != nullptr) {
        gtk_widget_destroy(GTK_WIDGET(window_termins));
        window_termins = nullptr;
    }

    // Завершить выполнение программы
    gtk_main_quit();
}

std::string UrlDecode(const std::string& encoded) {
    CURL* curl = curl_easy_init();

    int out_length;
    char* decoded = curl_easy_unescape(curl, encoded.c_str(), encoded.length(), &out_length);

    std::string decoded_string(decoded, out_length);
    curl_free(decoded);
    curl_easy_cleanup(curl);
    
    return decoded_string;
}

std::string RemoveNewlines(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c != '\n' && c != '\r') {
            result += c;
        }
    }
    return result;
}

static void on_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_data) {


    if (const guchar *selection_data_bytes = gtk_selection_data_get_data(data)) {
        
        size_t selection_data_length = gtk_selection_data_get_length(data);

        std::string url_str(reinterpret_cast<const char*>(selection_data_bytes), selection_data_length);


        std::string newString = UrlDecode(url_str);

        if (newString.compare(0, 7, "file://") == 0) {
            newString.erase(0, 7);
        }
        
        newString.erase(std::remove(newString.begin(), newString.end(), '\n'), newString.end());
        newString.erase(std::remove(newString.begin(), newString.end(), '\r'), newString.end());

        std::string output = RemoveNewlines(newString);
 
        #ifdef _WIN32
        while (!newString.empty() && newString[0] == ' ') {
            newString.erase(0, 1);
        }
        if (newString.compare(0, 1, "/") == 0) {
            newString.erase(0, 1);   
        }
        newString.erase(newString.size() - 1);
        folder_path = newString;

        #else
        std::cout << "folder_path: " << newString << std::endl;
		std::cout << "folder_path exists: " << std::filesystem::exists(newString) << std::endl;
		std::cout << "folder_path is_directory: " << std::filesystem::is_directory(newString) << std::endl;
        #endif

        if (std::filesystem::is_directory(folder_path)) {
			g_print("DND ");
            std::cout << folder_path << std::endl;
            dnd_flag = false;
            startLoading();
        }
    }
}

int main(int argc, char *argv[]) {

    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif
    gtk_init(&argc, &argv);
    builder = gtk_builder_new_from_file(gladeFilePath.c_str());
    
    window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    gtk_window_set_title(GTK_WINDOW(window1), "РЕТОМЕТР-М3");

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

    GtkTargetEntry target_entries[] = {
        {const_cast<gchar*>("text/uri-list"), 0, 0}
    };

    gtk_drag_dest_set(main_paned1, GTK_DEST_DEFAULT_ALL, target_entries, G_N_ELEMENTS(target_entries), GDK_ACTION_COPY);
    g_signal_connect(main_paned1, "drag-data-received", G_CALLBACK(on_drag_data_received), NULL);

    g_signal_connect(open_item_subitem, "activate", G_CALLBACK(open_file), window1);
    g_signal_connect(export_excel_subitem, "activate", G_CALLBACK(export_to_excel), window1);
    g_signal_connect(about_termins_subitem, "activate", G_CALLBACK(about_termins), window1);
    g_signal_connect(about_program_subitem, "activate", G_CALLBACK(about_program), window1);
    g_signal_connect(window1, "destroy", G_CALLBACK(on_main_window_destroy), NULL);

    g_signal_connect(tv1, "button-press-event", G_CALLBACK(create_scrollable_table), NULL);

    gtk_builder_connect_signals(builder, nullptr);
    gtk_widget_show_all(window1);
    gtk_main();
    g_object_unref(builder);

    return EXIT_SUCCESS;
}
