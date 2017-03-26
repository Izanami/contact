#ifndef CONTACT_WIN_H_
#define CONTACT_WIN_H_

#include "contactapp.h"
#include "gtk.h"

#define CONTACT_APP_WINDOW_TYPE (contact_app_window_get_type())
G_DECLARE_FINAL_TYPE(ContactAppWindow, contact_app_window, CONTACT, APP_WINDOW,
                     GtkApplicationWindow)

ContactAppWindow *contact_app_window_new(ContactApp *app);
void contact_app_window_open(ContactAppWindow *win, GFile *file);

#endif /* CONTACT_WIN_H_ */
