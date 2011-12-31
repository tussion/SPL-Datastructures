#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_spl_ds.h"
#include "spl_ds_collection.h"
#include "spl_ds_dll.h"

zend_class_entry *spl_ds_ce_Queue;
zend_object_handlers spl_ds_handlers_Queue;

typedef struct _spl_ds_queue_object {
    zend_object  std;
    spl_ds_dll  *list;
} spl_ds_queue_object;

#define SPL_DS_QUEUE_GET_LIST() \
    ((spl_ds_queue_object *) zend_object_store_get_object(getThis() TSRMLS_CC))->list

static void spl_ds_queue_free_storage(void *object TSRMLS_DC)
{
    spl_ds_queue_object *obj = (spl_ds_queue_object *) object;

    spl_ds_dll_free(obj->list);

    zend_object_std_dtor(&obj->std TSRMLS_CC);

    efree(obj);
}

static void spl_ds_queue_clone_storage(void *object, void **target_ptr)
{
    spl_ds_queue_object *obj_orig, *obj_clone;

    obj_orig = (spl_ds_queue_object *) object;
    obj_clone = (spl_ds_queue_object *) emalloc(sizeof(spl_ds_queue_object));

    memcpy(obj_clone, obj_orig, sizeof(spl_ds_queue_object));

    obj_clone->list = spl_ds_dll_clone(obj_orig->list);

    *target_ptr = obj_clone;
}

static zend_object_value spl_ds_queue_create_handler(zend_class_entry *class_type TSRMLS_DC)
{
    zend_object_value retval;
    spl_ds_queue_object *obj;

    obj = emalloc(sizeof(spl_ds_queue_object));

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->list = spl_ds_dll_create();

    retval.handle = zend_objects_store_put(obj, NULL, spl_ds_queue_free_storage, spl_ds_queue_clone_storage TSRMLS_CC);
    retval.handlers = &spl_ds_handlers_Queue;

    return retval;
}

SPL_DS_METHOD(Queue, peek)
{
    spl_ds_dll *list;
    zval *item;

    list = SPL_DS_QUEUE_GET_LIST();

    if (spl_ds_dll_is_empty(list)) {
        // TODO: throw exception
        return;
    }

    item = spl_ds_dll_get_first(list);
    RETURN_ZVAL(item, 1, 1);
}

SPL_DS_METHOD(Queue, pop)
{
    spl_ds_dll *list;
    zval *item;

    list = SPL_DS_QUEUE_GET_LIST();

    if (spl_ds_dll_is_empty(list)) {
        // TODO: throw exception
        return;
    }

    item = spl_ds_dll_remove_first(list);
    RETURN_ZVAL(item, 1, 1);
}

SPL_DS_METHOD(Queue, push)
{
    zval *item;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &item) == FAILURE) {
        return;
    }

    spl_ds_dll_add_last(SPL_DS_QUEUE_GET_LIST(), item);
}

SPL_DS_METHOD(Queue, clear)
{
    spl_ds_dll_clear(SPL_DS_QUEUE_GET_LIST());
}

SPL_DS_METHOD(Queue, isEmpty)
{
    if (spl_ds_dll_is_empty(SPL_DS_QUEUE_GET_LIST())) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

SPL_DS_METHOD(Queue, toArray)
{
    zval *retval = spl_ds_dll_to_array(SPL_DS_QUEUE_GET_LIST());

    RETURN_ZVAL(retval, 1, 1);
}


SPL_DS_METHOD(Queue, count)
{
    long count = spl_ds_dll_count(SPL_DS_QUEUE_GET_LIST());

    RETURN_LONG(count);
}

ZEND_BEGIN_ARG_INFO(spl_ds_arg_info_Queue_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(spl_ds_arg_info_Queue_push, 0, 0, 1)
    ZEND_ARG_INFO(0, item)
ZEND_END_ARG_INFO()

const zend_function_entry spl_ds_methods_Queue[] = {
    SPL_DS_ME(Queue, peek,    spl_ds_arg_info_Queue_void)
    SPL_DS_ME(Queue, pop,     spl_ds_arg_info_Queue_void)
    SPL_DS_ME(Queue, push,    spl_ds_arg_info_Queue_push)
    SPL_DS_ME(Queue, clear,   spl_ds_arg_info_Queue_void)
    SPL_DS_ME(Queue, isEmpty, spl_ds_arg_info_Queue_void)
    SPL_DS_ME(Queue, toArray, spl_ds_arg_info_Queue_void)
    SPL_DS_ME(Queue, count,   spl_ds_arg_info_Queue_void)
    PHP_FE_END
};

PHP_MINIT_FUNCTION(spl_ds_queue)
{
    SPL_DS_REGISTER_CLASS(Queue, spl_ds_queue_create_handler);
    zend_class_implements(spl_ds_ce_Queue TSRMLS_CC, 1, spl_ds_ce_Collection);

    memcpy(
        &spl_ds_handlers_Queue,
        zend_get_std_object_handlers(),
        sizeof(zend_object_handlers)
    );
    spl_ds_handlers_Queue.clone_obj = zend_objects_store_clone_obj;

    return SUCCESS;
}