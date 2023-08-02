#include <gtk-3.0/gtk/gtk.h>
#include <xlsxwriter.h>
#include <glib.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>
#include <ctime>
#include "pugixml.hpp"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif


GtkBuilder *builder;
//--------------------------------------//
// Создание некоторых объектов главного окна
GtkWidget *window1;
GtkWidget *window2;
GtkWidget *main_paned1;
GtkWidget *fixed1;
GtkWidget *fixed2;


//--------------------------------------//
// Получаем тестовый label
GtkWidget *label1;
GtkWidget *labelabout;

//--------------------------------------//
// Получаем кнопки
GtkWidget *button1;
GtkWidget *button2;
GtkWidget *button3;
GtkWidget *button6;

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
GtkTreeSelection *selection;

//--------------------------------------//
// Получаем столбцы в таблице
GtkCellRenderer *cr1;
GtkCellRenderer *cr2;
GtkCellRenderer *cr3;
GtkCellRenderer *cr4;
GtkCellRenderer *cr5;

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


// Функция для экспорта данных из GtkListStore в файл Excel
void on_button2_clicked(GtkButton *button, gpointer user_data) {


    GtkTreeModel *model = GTK_TREE_MODEL(liststoreResult);
    GtkTreeIter iter;

    // Создаем новый файл Excel
    lxw_workbook *workbook = workbook_new("output.xlsx");
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    // Получаем количество строк и столбцов в GtkListStore
    gint num_rows = gtk_tree_model_iter_n_children(model, NULL);
    gint num_columns = gtk_tree_model_get_n_columns(model);

// Инициализируем массив max_column_width нулями
    gint max_column_width[num_columns];
    memset(max_column_width, 0, sizeof(max_column_width));

//    std::cout << num_columns << std::endl;


    // Записываем заголовки столбцов в Excel
    for (int col = 0; col < num_columns; col++) {
        GtkTreeViewColumn *column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), col);
        const gchar *column_name = gtk_tree_view_column_get_title(column);
        worksheet_write_string(worksheet, 0, col, column_name, NULL);
    }

    // Записываем данные из GtkListStore в Excel
    for (int row = 0; row < num_rows; row++) {
        gtk_tree_model_iter_nth_child(model, &iter, NULL, row);

        for (int col = 0; col < num_columns; col++) {
            GValue value = G_VALUE_INIT;
            gtk_tree_model_get_value(model, &iter, col, &value);
            const gchar* str_value = g_value_get_string(&value);

            worksheet_write_string(worksheet, row + 1, col, str_value, NULL);

            int value_width = g_utf8_strlen(str_value, -1);

            if (value_width > max_column_width[col]) {
                max_column_width[col] = value_width;
                worksheet_set_column(worksheet, col, col, value_width + 3, NULL);
            }

            g_value_unset(&value);
        }
    }

    // Закрываем файл Excel и освобождаем ресурсы
    if (workbook_close(workbook) != LXW_NO_ERROR) {
        g_printerr("Ошибка при сохранении файла Excel.\n");
    }

    #ifdef _WIN32
        // Для Windows используем команду "start" для открытия файла через ассоциированное приложение
        ShellExecuteA(NULL, "open", folder_path, NULL, NULL, SW_SHOWNORMAL); //"output.xlsx"
    #elif __linux__
        // Для Linux используем команду "xdg-open" для открытия файла через ассоциированное приложение
        if (fork() == 0) {
            execlp("xdg-open", "xdg-open", "output.xlsx", NULL);
        }
    #endif
}

void on_renderer_clicked() {
    // Устанавливаем новый текст для label1
    gtk_label_set_text(GTK_LABEL(label1), "НАЖАЛ НА 2 КНОПКУ!!!");
}

void create_scrollable_table(GtkButton *button, gpointer user_data) {

    main_paned1 = GTK_WIDGET(gtk_builder_get_object(builder, "main_paned1"));
//    GtkWidget *upper_paned = gtk_paned_get_child1(GTK_PANED(main_paned1));
//    GtkWidget *lower_paned = gtk_paned_get_child2(GTK_PANED(main_paned1));
    // Задаем фиксированную позицию для верхней части
    gtk_paned_set_position(GTK_PANED(main_paned1), 200); // Установите здесь желаемую фиксированную высоту

    GtkWidget *scrolled2 = gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);


// Добавление GtkScrolledWindow в нижний GtkPaned
    gtk_paned_pack2(GTK_PANED(main_paned1), scrolled2, TRUE, TRUE);

    // Установка таблицы в контейнер с прокруткой
    gtk_container_add(GTK_CONTAINER(scrolled2), treeview);

    // Добавление GtkScrolledWindow в нижний GtkPaned
    gtk_paned_pack2(GTK_PANED(main_paned1), scrolled2, TRUE, TRUE);

    gtk_widget_show_all(window1);

}

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
std::string getDirectoryFromPath(const std::string &path) {
    size_t lastSlashPos = path.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        return path.substr(0, lastSlashPos + 1);
    }
    return "";
}

// Функция для загрузки содержимого файла в память
GBytes *loadFileContents(const gchar *filename) {
    GError *error = NULL;
    gsize fileSize;
    gchar *fileContents = NULL;

    if (!g_file_get_contents(filename, &fileContents, &fileSize, &error)) {
        g_print("Failed to load file '%s': %s\n", filename, error->message);
        g_error_free(error);
        return NULL;
    }

    GBytes *bytes = g_bytes_new_static(fileContents, fileSize);

    return bytes;
}

//--------------------------------------//
//обработчик события для кнопки button6
void on_button6_clicked(GtkButton *button, GtkWidget *label1, gpointer user_data) {
    gtk_widget_destroy(window2);
    g_object_unref(window2);
}

//--------------------------------------//
//обработчик события для кнопки button3
void on_button3_clicked(GtkButton *button, gpointer user_data) {
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

//--------------------------------------//
//обработчик события для кнопки button1
void on_button1_clicked(GtkButton *button, gpointer user_data) {
//    GtkBuilder    *builder = GTK_BUILDER(user_data);
    GtkWidget *dialog1;

    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

    gint res;
    int k = 0;
//
    std::string name_object;
    std::string start_reg;
    std::string schematic_connect;
    std::string average_interval;
    std::string end_reg;

    std::vector<std::string> paramAttrInBlock;
    std::vector<std::string> resultAttrInBlock;

    std::string timeStart;
    std::string timeStop;
    std::string nameObject;
    std::string averagingIntervalTime;
    std::string averagingInterval;
    std::string activeCxema;

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

        // Делаем кнопку активной снова
        gtk_widget_set_sensitive(button2, TRUE);

        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog1);
        folder_path = gtk_file_chooser_get_filename(chooser);

        int numColumns = 33; // Количество столбцов

        // Создание таблицы
        treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(liststoreResult));
        renderer = gtk_cell_renderer_text_new();
        gtk_list_store_append(liststoreResult, &iterResult);
        std::vector<const char *> values(numColumns);

        // Устанавливаем модель и режим автоматического сокрытия пустых строк
//        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(liststoreResult));
//        gtk_list_store_set_value(GTK_LIST_STORE(liststoreResult), GTK_TREE_MODEL_SHOW_EMPTY);

        // Вместо создания отдельных переменных для каждой колонки, можно использовать массив указателей на GtkWidget:
        GtkWidget *columns[numColumns];

        gchar *titles[] = {
                "Время", "Uab, B", "Ubc, B", "Uca, B", "Ia, A", "Ib, A", "Ic, A",
                "Ua, B", "Ub, B", "Uc, B", "Pп, Вт", "Pо, Вт", "Pн, Вт", "Qп, Вар",
                "Qо, Вар", "Qн, Вар", "Sп, ВА", "Sо, ВА", "Sн, ВА", "Uп, В", "Uо, В",
                "Uн, В", "Iп, А", "Iо, А", "Iн, А", "Kо", "Kн", "△f, Гц",
                "△Uy, %", "△UyA, %", "△UyB, %", "△UyC, %"
        }; // TODO проверить столбцы
        for (int i = 0; i < numColumns; ++i) {
//            if (!columnsCreated) {
            columns[i] = reinterpret_cast<GtkWidget *>(gtk_tree_view_column_new());
            gchar *title = g_strdup_printf("%s", titles[i]);
            gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(columns[i]), title);
            g_free(title);

//            }
            gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(columns[i]), renderer, TRUE);
            gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(columns[i]), renderer, "text", i);

            gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), reinterpret_cast<GtkTreeViewColumn *>(columns[i]));
        }

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
                // здесь надо реализовать заполнение одной строки
                // Добавляем новую строку
                gtk_list_store_append(liststoreResult, &iterResult);

                for (int i = 1; i < numColumns; ++i) {
                    values[i] = g_strdup_printf("%s", resultAttrInBlock[i].c_str());
                }

                //TimeTek
                std::string time_tek = values[1];
                float numberstart = std::stof(time_tek);  // Преобразуем строку в число типа float
                // Создаем структуру tm с помощью значения UNIX времени
                struct tm *timeket_tm;
                numberstart /= 1000;
                // Преобразуем значение float в тип time_t
                time_t timeValue = static_cast<time_t>(numberstart);
                timeket_tm = localtime(reinterpret_cast<const time_t *>(&timeValue));
                // Преобразуем структуру tm в строку с помощью функции strftime
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeket_tm);
                const char *formattedTek(buffer);
                gtk_list_store_set(liststoreResult, &iterResult,
                                   0, formattedTek,
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

                // здесь надо реализовать заполнение одной строки
//-----------------------------------------------------------------------------------------------------------------------------------------
                resultAttrInBlock.clear();

            }

            if (k == 0) {
//              Наименование объекта
//                name_object = processedData[2];
                name_object = paramAttrInBlock[2];

//              Старт испытания
                start_reg = paramAttrInBlock[0];
                float numberstart = std::stof(start_reg);  // Преобразуем строку в число типа float
                // Создаем структуру tm с помощью значения UNIX времени
                struct tm *timeinfo;
                numberstart /= 1000;
                // Преобразуем значение float в тип time_t
                time_t timeValue = static_cast<time_t>(numberstart);
                timeinfo = localtime(reinterpret_cast<const time_t *>(&timeValue));
                // Преобразуем структуру tm в строку с помощью функции strftime
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
                std::string formattedTimeStart(buffer);

//              Окончание испытания
                end_reg = paramAttrInBlock[1];
                float numberend = std::stof(end_reg);  // Преобразуем строку в число типа float
                // Создаем структуру tm с помощью значения UNIX времени
                struct tm *timeinfoend;
                numberend /= 1000;
                // Преобразуем значение float в тип time_t
                time_t timeValueend = static_cast<time_t>(numberend);
                timeinfoend = localtime(reinterpret_cast<const time_t *>(&timeValueend));
                // Преобразуем структуру tm в строку с помощью функции strftime
                char bufferend[80];
                strftime(bufferend, sizeof(bufferend), "%Y-%m-%d %H:%M:%S", timeinfoend);
                std::string formattedTimeEnd(bufferend);
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
//                GtkListStore *liststore = GTK_LIST_STORE(user_data);
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

            }
            k++;

//            std::cout << size(resultAttrInBlock) << std::endl;
//            std::string pke_cxema = resultAttrInBlock[0];
//            std::string TimeTek = resultAttrInBlock[1];
        }
        g_free(folder_path);
    }
    g_signal_connect(tv1, "row-activated", G_CALLBACK(create_scrollable_table), NULL);



    gtk_widget_destroy(dialog1);
}

// Обработчик сигнала для события "destroy" окна window2
void on_window2_destroy(GtkWidget *widget, gpointer user_data) {
    // Закрытие окна window2
    gtk_widget_destroy(GTK_WIDGET(user_data));
    // Выгрузка из памяти builder2 и других связанных объектов
    g_object_unref(builder);
}


//--------------------------------------//
int main(int argc, char *argv[]) {

    while (true) {
        // Инициализация GTK
        gtk_init(&argc, &argv);

        // Получение пути к исполняемому файлу
        std::string executablePath = getExecutablePath();
        // Получение директории из пути к исполняемому файлу
        std::string executableDirectory = getDirectoryFromPath(executablePath);
        // Формирование пути к файлу glade.glade (абсолютный или относительный относительно исполняемого файла)
        std::string gladeFilePath = executableDirectory + "glade.glade";

        // Загрузка интерфейса из файла glade.glade
        builder = gtk_builder_new_from_file(gladeFilePath.c_str());

        window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
        gtk_window_set_title(GTK_WINDOW(window1), "РЕТОМЕТР-М3");
        fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));

        //--------------------------------------//
        label1 = GTK_WIDGET(gtk_builder_get_object(builder, "label1"));

        button1 = GTK_WIDGET(gtk_builder_get_object(builder, "button1"));

        button2 = GTK_WIDGET(gtk_builder_get_object(builder, "button2"));
        // Делаем кнопку неактивной
        gtk_widget_set_sensitive(button2, FALSE);

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

//        gtk_list_store_append(liststore1, &iter);
//        gtk_list_store_set(liststore1, &iter, 0, " ", -1);
//        gtk_list_store_set(liststore1, &iter, 1, " ", -1);
//        gtk_list_store_set(liststore1, &iter, 2, " ", -1);
//        gtk_list_store_set(liststore1, &iter, 3, " ", -1);
//        gtk_list_store_set(liststore1, &iter, 4, " ", -1);

//        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv1));
//        // Установка режима выделения строк
//        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
        // Подписка на сигнал "changed"
//        g_signal_connect(selection, "row_activated", G_CALLBACK(on_row_activated), NULL);

        // Подключение обработчиков событий
        g_signal_connect(button1, "clicked", G_CALLBACK(on_button1_clicked), liststore1);
        g_signal_connect(button2, "clicked", G_CALLBACK(on_button2_clicked), window1);
        g_signal_connect(button3, "clicked", G_CALLBACK(on_button3_clicked), window1);

        g_signal_connect(window1, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        g_signal_connect(window2, "destroy", G_CALLBACK(gtk_main_quit), NULL);

        gtk_builder_connect_signals(builder, nullptr);
        gtk_widget_show_all(window1);
        gtk_main();
        g_object_unref(builder);

        if (gtk_main_quit) { break; }
    }
    return EXIT_SUCCESS;
}
