#ifndef CONTACT_TREE_H_
#define CONTACT_TREE_H_

#include "gtk.h"

#define CONTACT_TREE_TYPE (contact_tree_get_type())
G_DECLARE_FINAL_TYPE(ContactTree, contact_tree, CONTACT, TREE, GtkListStore)

ContactTree *contact_tree_new(void);
gboolean contact_tree_open(ContactTree *, GFile *);
gboolean contact_tree_set_window(ContactTree *, GtkWindow *);
gboolean contact_tree_line(ContactTree *, char *);
gboolean contact_tree_firstname(ContactTree *, GtkTreeIter *, char *);
gboolean contact_tree_lastname(ContactTree *, GtkTreeIter *, char *);
gboolean contact_tree_mail(ContactTree *, GtkTreeIter *, char *);
gboolean contact_tree_phone(ContactTree *, GtkTreeIter *, char *);
gboolean contact_tree_birth(ContactTree *, GtkTreeIter *, char *);
void contact_tree_capitalize(ContactTree *, char *);

enum {
    COLUMN_FIRSTNAME,
    COLUMN_LASTNAME,
    COLUMN_MAIL,
    COLUMN_PHONE,
    COLUMN_BIRTH,
    N_COLUMNS
};

#endif /* CONTACT_TREE_H_ */
