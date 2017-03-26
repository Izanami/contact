#ifndef CONTACT_APP_H_
#define CONTACT_APP_H_

#include "gtk.h"

#define CONTACT_APP_TYPE (contact_app_get_type())
G_DECLARE_FINAL_TYPE(ContactApp, contact_app, CONTACT, APP, GtkApplication)

ContactApp *contact_app_new(void);

#endif /* CONTACT_APP_H_ */
