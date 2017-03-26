#ifndef CONTACT_TREE_H_
#define CONTACT_TREE_H_

#include "gtk.h"

#define CONTACT_TREE_TYPE (contact_tree_get_type())
G_DECLARE_FINAL_TYPE(ContactTree, contact_tree, CONTACT, TREE, GtkListStore)

ContactTree *contact_tree_new(void);
gboolean contact_tree_open(ContactTree *, GFile *);

enum {
    COLUMN_FIRSTNAME,
    COLUMN_LASTNAME,
    COLUMN_MAIL,
    COLUMN_PHONE,
    COLUMN_BIRTH,
    N_COLUMNS
};

#endif /* CONTACT_TREE_H_ */
