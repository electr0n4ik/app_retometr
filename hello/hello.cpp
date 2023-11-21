#include <gtk/gtk.h>

static void on_drag_motion(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer user_data)
{
    g_print("Mouse motion at (%d, %d)\n", x, y);
}

static void on_drag_enter(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_data)
{
    g_print("Drag enter at (%d, %d)\n", x, y);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_title(GTK_WINDOW(window), "Drag and Drop Example");

    GtkWidget *event_box = gtk_event_box_new();  // Используем GtkEventBox в качестве цели
    gtk_widget_set_size_request(event_box, 200, 200);  // Устанавливаем размер виджета

    GtkWidget *label = gtk_label_new("Drop files here!");

    gtk_container_add(GTK_CONTAINER(event_box), label);
    gtk_container_add(GTK_CONTAINER(window), event_box);

    // Устанавливаем виджет event_box как цель для перетаскивания
    gtk_drag_dest_set(event_box, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);

    // Подключаем обработчики событий для motion и enter
    g_signal_connect(window, "drag-motion", G_CALLBACK(on_drag_motion), NULL);
    g_signal_connect(window, "drag-enter", G_CALLBACK(on_drag_enter), NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
