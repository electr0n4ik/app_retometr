#include <gtk-3.0/gtk/gtk.h>
#include <stdlib.h>
#include <iostream>
#include <filesystem>
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
// Получаем кнопки главного окна
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



//--------------------------------------//
//обработчик события для кнопки button2
void on_button2_clicked(GtkButton *button, gpointer user_data)
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
    builder = gtk_builder_new_from_file("test1.glade");
    window2 = GTK_WIDGET(gtk_builder_get_object(builder, "window2"));
    gtk_window_set_title(GTK_WINDOW(window2), "О программе");

    //gtk_window_set_resizable(GTK_WINDOW(window2));
    //gtk_window_activate_focus(GTK_WINDOW(window2));

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
void on_button1_clicked(GtkButton *button, gpointer user_data)
{
    GtkBuilder    *builder = GTK_BUILDER(user_data);
    GtkWidget     *dialog1;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    gint res;
    dialog1 = gtk_file_chooser_dialog_new("Выбрать папку",
                                          GTK_WINDOW(user_data),
                                          action,
                                          ("Отмена"),
                                          GTK_RESPONSE_CANCEL,
                                          ("Выбрать"),
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog1));

    // Если пользователь выбрал папку, то вывести ее путь в label
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

            // вывод содержимого корневого элемента файла
            // Устанавливаем новый текст для label1
            gtk_label_set_text(GTK_LABEL(label1), folder_path);

            std::ostringstream ss;
            doc.print(ss);
            std::string str = ss.str();
            gtk_label_set_text(GTK_LABEL(label1), str.c_str());
            std::cout << std::endl;
            std::cout << "\n\n";

        }
        g_free(folder_path);
    }
    gtk_widget_destroy(dialog1);
}

//--------------------------------------//
int main (int argc, char *argv[]) {

    while(true)

    {
        gtk_init(&argc, &argv);

        builder = gtk_builder_new_from_file("test1.glade");
        window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
        gtk_window_set_title(GTK_WINDOW(window1), "РЕТОМЕТР-М3");
        fixed1 = GTK_WIDGET(gtk_builder_get_object(builder, "fixed1"));

        //--------------------------------------//
        label1 = GTK_WIDGET(gtk_builder_get_object(builder, "label1"));

        button1 = GTK_WIDGET(gtk_builder_get_object(builder, "button1"));
        button2 = GTK_WIDGET(gtk_builder_get_object(builder, "button2"));
        button3 = GTK_WIDGET(gtk_builder_get_object(builder, "button3"));

        view1 = GTK_WIDGET(gtk_builder_get_object(builder, "view1"));
        liststore1 = GTK_LIST_STORE(gtk_builder_get_object(builder, "liststore1"));
        tv1 = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tv1"));

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

        selection = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "selection"));

        gtk_tree_view_column_add_attribute(cx1, cr1, "text", 0);
        gtk_tree_view_column_add_attribute(cx2, cr2, "text", 1);
        gtk_tree_view_column_add_attribute(cx3, cr3, "text", 2);
        gtk_tree_view_column_add_attribute(cx4, cr4, "text", 3);
        gtk_tree_view_column_add_attribute(cx5, cr5, "text", 4);

        GtkTreeIter iter;  // iterators

        gtk_list_store_append(liststore1, &iter);
        gtk_list_store_set(liststore1, &iter, 0, "row 1 data", -1);
        gtk_list_store_set(liststore1, &iter, 1, "row 1 data", -1);
        gtk_list_store_set(liststore1, &iter, 2, "row 1 data", -1);
        gtk_list_store_set(liststore1, &iter, 3, "row 1 data", -1);
        gtk_list_store_set(liststore1, &iter, 4, "row 1 data", -1);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv1));

        // Подключение обработчиков событий
        g_signal_connect(button1, "clicked", G_CALLBACK(on_button1_clicked), window1);
        g_signal_connect(button2, "clicked", G_CALLBACK(on_button2_clicked), NULL);
        g_signal_connect(button3, "clicked", G_CALLBACK(on_button3_clicked), window1);

        g_signal_connect(window1, "destroy", G_CALLBACK(gtk_main_quit), NULL);

        gtk_builder_connect_signals(builder, NULL);

        gtk_widget_show_all(window1);

        gtk_main();

        g_object_unref(builder);
        if (gtk_main_quit){break;}
    }
    return EXIT_SUCCESS;
}
