#include <stdatomic.h>
#include <algorithms/linkedlist.h>
#include <libutils/id_mapper.h>

id_mapper_t* id_mapper_create(id_mapper_t* id_mapper)
{
    if (!id_mapper) {
        id_mapper = (id_mapper_t*) malloc(sizeof(id_mapper_t));
    }
    id_mapper->list = linkedlist_create();
    id_mapper->next = ATOMIC_VAR_INIT(1);
    return id_mapper;
}

id_handler_t id_mapper_next_id(id_mapper_t* id_mapper)
{
    if (!id_mapper) {
        return 0;
    }

    // with lock support
    id_handler_t next = atomic_fetch_add(&id_mapper->next, 1);
    return next;
}

void* id_mapper_get(id_mapper_t* id_mapper, id_handler_t id)
{
    if (!id_mapper || !id) {
        return NULL;
    }
    WHILE_LINKEDLIST_ITER(id_mapper->list, id_mapper_entry_t*, entry) {
        if (entry->id == id) {
            return entry->val;
        }
    }
    return NULL;
}

id_handler_t id_mapper_add(id_mapper_t* id_mapper, void* val)
{
    if (!id_mapper || !val) {
        //we don't store NULL values because it is impossible to know the different if id_mapper_get
        return 0;
    }

    id_mapper_entry_t* entry = (id_mapper_entry_t*) malloc(sizeof(id_mapper_entry_t));
    entry->id = id_mapper_next_id(id_mapper);
    entry->val = val;
    linkedlist_append(id_mapper->list, entry);
    return entry->id;
}

static int _find_id(void* id, void* entry)
{
    if (((id_mapper_entry_t*) entry)->id == *(id_handler_t*) id) {
        return ITER_REMOVE_AND_RETURN;
    }
    return ITER_GO_NEXT;
}

bool id_mapper_del(id_mapper_t* id_mapper, id_handler_t id)
{
    if (!id_mapper || !id) {
        return false;
    }

    return linkedlist_iter_with_action(id_mapper->list, _find_id, &id) != NULL;
}
