#include <gtk-3.0/gtk/gtk.h>
#include <glib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>
#include <ctime>
#include "pugixml.hpp"

GtkBuilder               *builder;
//--------------------------------------//
// Создание некоторых объектов главного окна
GtkWidget                *window1;
GtkWidget                *window2;
GtkWidget                *fixed1;
GtkWidget                *fixed2;

//--------------------------------------//
// Получаем тестовый label
GtkWidget                *label1;
GtkWidget                *labelabout;

//--------------------------------------//
// Получаем кнопки
GtkWidget                *button1;
GtkWidget                *button2;
GtkWidget                *button3;
GtkWidget                *button6;

//--------------------------------------//
// Получаем таблицу в главном окне
// для вывода информации
// о загруженных файлах ПКЭ
GtkWidget                *view1;
GtkListStore             *liststore1;
GtkTreeView              *tv1;
GtkTreeViewColumn        *cx1;
GtkTreeViewColumn        *cx2;
GtkTreeViewColumn        *cx3;
GtkTreeViewColumn        *cx4;
GtkTreeViewColumn        *cx5;
GtkTreeSelection         *selection;

//--------------------------------------//
// Получаем столбцы в таблице
GtkCellRenderer          *cr1;
GtkCellRenderer          *cr2;
GtkCellRenderer          *cr3;
GtkCellRenderer          *cr4;
GtkCellRenderer          *cr5;

// Функция для получения пути к исполняемому файлу
std::string getExecutablePath() {
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
        return std::string(path);
    }
    return "";
}

// Функция для получения директории из пути
std::string getDirectoryFromPath(const std::string& path) {
    size_t lastSlashPos = path.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        return path.substr(0, lastSlashPos + 1);
    }
    return "";
}

// Функция для загрузки содержимого файла в память
GBytes* loadFileContents(const gchar* filename) {
    GError* error = NULL;
    gsize fileSize;
    gchar* fileContents = NULL;

    if (!g_file_get_contents(filename, &fileContents, &fileSize, &error)) {
        g_print("Failed to load file '%s': %s\n", filename, error->message);
        g_error_free(error);
        return NULL;
    }

    GBytes* bytes = g_bytes_new_static(fileContents, fileSize);

    return bytes;
}

//--------------------------------------//
//обработчик события для кнопки button2
void on_button2_clicked()
{
    // Устанавливаем новый текст для label1
    gtk_label_set_text(GTK_LABEL(label1), "НАЖАЛ НА 2 КНОПКУ!!!");
}

//--------------------------------------//
//обработчик события для кнопки button6
void on_button6_clicked(GtkButton *button, GtkWidget *label1, gpointer user_data)
{
    gtk_widget_destroy(window2);
    g_object_unref(window2);
}

//--------------------------------------//
//обработчик события для кнопки button3
void on_button3_clicked(GtkButton *button, gpointer user_data)
{
    builder = gtk_builder_new_from_file("glade.glade");
    window2 = GTK_WIDGET(gtk_builder_get_object(builder, "window2"));
    gtk_window_set_title(GTK_WINDOW(window2), "О программе");

    fixed2 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed2"));
    labelabout = GTK_WIDGET(gtk_builder_get_object(builder, "labelabout"));
    button6 = GTK_WIDGET(gtk_builder_get_object(builder, "button6"));

    // Устанавливаем новый текст для label1
    gtk_label_set_text(GTK_LABEL(label1), "НАЖАЛ НА 3 КНОПКУ!!!");

    g_signal_connect(button6, "clicked", G_CALLBACK(on_button6_clicked), window2);

    gtk_widget_show_all(window2);
    //g_object_unref(G_OBJECT(builder));
}

std::vector<std::string> extractDataFromXML(const std::string& xmlText) {
    std::regex timeStartRegex("TimeStart=\"(\\d+)\"");
    std::regex timeStopRegex("TimeStop=\"(\\d+)\"");
    std::regex nameObjectRegex("nameObject=\"([^\"]+)\"");
    std::regex averagingIntervalTimeRegex("averaging_interval_time=\"(\\d+)\"");
    std::regex averagingIntervalRegex("averaging_interval=\"(\\d+)\"");
    std::regex activeCxemaRegex("active_cxema=\"(\\d+)\"");

    std::vector<std::string> extractedData;

    std::smatch match;

    // Поиск кода времени TimeStart
    if (std::regex_search(xmlText, match, timeStartRegex)) {
        extractedData.push_back(match[1].str());
    }

    // Поиск кода времени TimeStop
    if (std::regex_search(xmlText, match, timeStopRegex)) {
        extractedData.push_back(match[1].str());
    }

    // Поиск значения nameObject
    if (std::regex_search(xmlText, match, nameObjectRegex)) {
        extractedData.push_back(match[1].str());
    }

    // Поиск значения averaging_interval_time
    if (std::regex_search(xmlText, match, averagingIntervalTimeRegex)) {
        extractedData.push_back(match[1].str());
    }

    // Поиск значения averaging_interval
    if (std::regex_search(xmlText, match, averagingIntervalRegex)) {
        extractedData.push_back(match[1].str());
    }

    // Поиск значения active_cxema
    if (std::regex_search(xmlText, match, activeCxemaRegex)) {
        extractedData.push_back(match[1].str());
    }

    return extractedData;
}

//--------------------------------------//
//обработчик события для кнопки button1
void on_button1_clicked(GtkButton *button, gpointer user_data)
{
    GtkBuilder    *builder = GTK_BUILDER(user_data);
    GtkWidget     *dialog1;

    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

    gint res;
    int i = 0;

    std::string name_object;
    std::string start_reg;
    std::string schematic_connect;
    std::string average_interval;
    std::string end_reg;

    dialog1 = gtk_file_chooser_dialog_new("Выбрать папку",
                                          GTK_WINDOW(user_data),
                                          action,
                                          ("Отмена"),
                                          GTK_RESPONSE_CANCEL,
                                          ("Выбрать"),
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog1));

    // Если пользователь выбрал папку, то вывод в label
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog1);
        gchar *folder_path = gtk_file_chooser_get_filename(chooser);

        for (const auto& entry : std::filesystem::directory_iterator(folder_path))
        {

            // проход по всем файлам в папке
            std::string file_path = entry.path().string();
            pugi::xml_document doc;
            if (!doc.load_file(file_path.c_str()))
            {
                std::cerr << "Не удалось загрузить файл: " << file_path << std::endl;
                continue;
            }

            std::ostringstream ss;
            doc.print(ss);
            std::string str = ss.str();
            std::vector<std::string> processedData = extractDataFromXML(str);

            if (i == 0)
            {

                name_object = processedData[2];

//              Старт испытания
                start_reg = processedData[0];
                float numberstart = std::stof(start_reg);  // Преобразуем строку в число типа float
                // Создаем структуру tm с помощью значения UNIX времени
                struct tm* timeinfo;
                numberstart /= 1000;
                // Преобразуем значение float в тип time_t
                time_t timeValue = static_cast<time_t>(numberstart);
                timeinfo = localtime(reinterpret_cast<const time_t *>(&timeValue));
                // Преобразуем структуру tm в строку с помощью функции strftime
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
                std::string formattedTimeStart(buffer);

                //Окончание испытания
                end_reg = processedData[1];
                float numberend = std::stof(end_reg);  // Преобразуем строку в число типа float
                // Создаем структуру tm с помощью значения UNIX времени
                struct tm* timeinfoend;
                numberend /= 1000;
                // Преобразуем значение float в тип time_t
                time_t timeValueend = static_cast<time_t>(numberend);
                timeinfoend = localtime(reinterpret_cast<const time_t *>(&timeValueend));
                // Преобразуем структуру tm в строку с помощью функции strftime
                char bufferend[80];
                strftime(bufferend, sizeof(bufferend), "%Y-%m-%d %H:%M:%S", timeinfoend);
                std::string formattedTimeEnd(bufferend);

                schematic_connect = processedData[5];
                average_interval = processedData[3];

                GtkListStore *liststore = GTK_LIST_STORE(user_data);

                // Удаление первой строки
//                GtkTreeIter first_row;
//                if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore), &first_row)) {
//                    gtk_list_store_remove(liststore, &first_row);
//                }
                gtk_list_store_clear(liststore1);

                GtkTreeIter iter;  // iterators

                gtk_list_store_append(liststore1, &iter);
                gtk_list_store_set(liststore1, &iter, 0, name_object.c_str(), -1); // Название объекта
                gtk_list_store_set(liststore1, &iter, 1, formattedTimeStart.c_str(), -1); // Начало регистрации
                gtk_list_store_set(liststore1, &iter, 2, formattedTimeEnd.c_str(), -1); // Окончание регистрации
                gtk_list_store_set(liststore1, &iter, 3, schematic_connect.c_str(), -1); // Схема соединения
                gtk_list_store_set(liststore1, &iter, 4, average_interval.c_str(), -1); // Интервал усреднения

                // Добавление новых пустых строк в таблицу
//                for (int i = 1; i < 10; i++) {
//
//                    gtk_list_store_append(liststore1, &iter);
//                    gtk_list_store_set(liststore1, &iter, 0, "", -1);
//                }

                std::cout << "\nНазвание объекта: " << name_object << "\nНачало регистрации:    " << start_reg;
                std::cout << "\nОкончание регистрации: " << end_reg << "\nСхема соединения: " << schematic_connect;
                std::cout << "\nИнтервал усреднения: " << average_interval << "\n";
                std::cout << std::endl; // Отладочный вывод
            }

            i += 1;
        }
        g_free(folder_path);
    }
    gtk_widget_destroy(dialog1);
}

// Обработчик сигнала для события "destroy" окна window2
void on_window2_destroy(GtkWidget *widget, gpointer user_data) {
    // Закрытие окна window2
    gtk_widget_destroy(GTK_WIDGET(user_data));
    // Выгрузка из памяти builder2 и других связанных объектов
    g_object_unref(builder);
}

// Создайте функцию-обработчик для сигнала "row-activated"
void on_table_clicked() {
    // Устанавливаем новый текст для label1
    gtk_label_set_text(GTK_LABEL(label1), "НАЖАЛ НА 22 КНОПКУ!!!");
}

// Обработчик события для выбора строки таблицы
void on_row_activated_1(GtkTreeView *treeview, gpointer user_data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
    GtkTreeModel *model;
    GtkTreeIter iter;

    // Получение выбранной строки
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        // Получение значения из первого столбца выбранной строки
        gchar *filename;
        gtk_tree_model_get(model, &iter, 0, &filename, -1);

        // Создание нового окна
        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Содержимое файла");
        gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

        // Создание таблицы в новом окне
        GtkWidget *table = gtk_grid_new();
        gtk_container_add(GTK_CONTAINER(window), table);

        // Чтение содержимого файла
        std::ifstream file(filename);
        std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // Создание многострочного текстового виджета для отображения содержимого файла
        GtkWidget *textview = gtk_text_view_new();
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_buffer_set_text(buffer, fileContent.c_str(), -1);
        gtk_grid_attach(GTK_GRID(table), textview, 0, 0, 1, 1);

        // Отображение нового окна
        gtk_widget_show_all(window);

        g_free(filename);
    }
}

//--------------------------------------//
int main (int argc, char *argv[]) {

    while(true)

    {
        // Инициализация GTK
        gtk_init(&argc, &argv);

        // Получение пути к исполняемому файлу
        std::string executablePath = getExecutablePath();
        // Получение директории из пути к исполняемому файлу
        std::string executableDirectory = getDirectoryFromPath(executablePath);
        // Формирование пути к файлу glade.glade (абсолютный или относительный относительно исполняемого файла)
        std::string gladeFilePath = executableDirectory + "glade.glade";

        // Загрузка интерфейса из файла glade.glade
        GtkBuilder *builder = gtk_builder_new_from_file(gladeFilePath.c_str());

        window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
        gtk_window_set_title(GTK_WINDOW(window1), "РЕТОМЕТР-М3");
        fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));

        //--------------------------------------//
        label1 = GTK_WIDGET(gtk_builder_get_object(builder, "label1"));

        button1 = GTK_WIDGET(gtk_builder_get_object(builder, "button1"));
        button2 = GTK_WIDGET(gtk_builder_get_object(builder, "button2"));
        button3 = GTK_WIDGET(gtk_builder_get_object(builder, "button3"));

        // Создание модели данных
        liststore1 = GTK_LIST_STORE(gtk_builder_get_object(builder, "liststore1"));
        tv1 = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tv1"));

        cx1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx1"));
        cx2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx2"));
        cx3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx3"));
        cx4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx4"));
        cx5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx5"));
//        // Установка фиксированной ширины для третьего столбца
//        gtk_tree_view_column_set_fixed_width(cx5, 300);

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

        // Получение размера окна
//        GtkAllocation allocation;
//        GtkWidget *tv1 = GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(builder), "tv1"));
//        gtk_widget_get_allocation(tv1, &allocation);
//        int height = allocation.height;

        // Вычисление количества строк, достаточного для заполнения окна
//        int row_height = 0;
//        GtkTreeModel *model = GTK_TREE_MODEL(liststore1);
//        GtkTreeIter iter;  // iterators
//        int row_count = 0;
//
//        while (gtk_tree_model_iter_next(model, &iter)) {
//            GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
//            GtkTreeView *tv1 = GTK_TREE_VIEW(builder);
//            GdkRectangle rect;
//            if (gtk_tree_view_get_background_area(tv1, path, NULL, &rect)) {
//                row_height = rect.height;
//            }
//            gtk_tree_path_free(path);
//            row_count++;
//        }
//        int max_rows = height / row_height + 1;
        GtkTreeIter iter;
        // Добавление новых пустых строк в таблицу
//        for (int i = 1; i < 5; i++) {
//
//            gtk_list_store_append(liststore1, &iter);
//            gtk_list_store_set(liststore1, &iter, 0, "", -1);
//        }

        gtk_list_store_append(liststore1, &iter);
        gtk_list_store_set(liststore1, &iter, 0, " ", -1);
        gtk_list_store_set(liststore1, &iter, 1, " ", -1);
        gtk_list_store_set(liststore1, &iter, 2, " ", -1);
        gtk_list_store_set(liststore1, &iter, 3, " ", -1);
        gtk_list_store_set(liststore1, &iter, 4, " ", -1);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv1));
        // Установка режима выделения строк
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
        // Подписка на сигнал "changed"
//        g_signal_connect(selection, "row_activated", G_CALLBACK(on_row_activated), NULL);


        // Подключение обработчиков событий
        g_signal_connect(button1, "clicked", G_CALLBACK(on_button1_clicked), liststore1);
        g_signal_connect(button2, "clicked", G_CALLBACK(on_button2_clicked), window1);
        g_signal_connect(button3, "clicked", G_CALLBACK(on_button3_clicked), window1);

        g_signal_connect(window1, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        g_signal_connect(window2, "destroy", G_CALLBACK(gtk_main_quit), NULL);

        // Подключение обработчика события клика на таблицу
        g_signal_connect(cr1, "row_selected", G_CALLBACK(on_table_clicked), NULL);


        gtk_builder_connect_signals(builder, NULL);
        gtk_widget_show_all(window1);
        gtk_main();
        g_object_unref(builder);

        if (gtk_main_quit){break;}
    }
    return EXIT_SUCCESS;
}
