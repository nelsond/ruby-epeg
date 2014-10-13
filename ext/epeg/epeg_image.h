#include <math.h>
#include <Epeg.h>
#include <ruby.h>

static VALUE rb_epeg_image_open(VALUE, VALUE);
static VALUE rb_epeg_image_from_blob(VALUE, VALUE);
static VALUE rb_epeg_image_get_default_quality(VALUE);
static VALUE rb_epeg_image_set_default_quality(VALUE, VALUE);

static VALUE rb_epeg_image_initialize(VALUE);

static void rb_epeg_image_encode_or_trim(VALUE, Epeg_Image *);
static VALUE rb_epeg_image_resize(VALUE, VALUE, VALUE);
static VALUE rb_epeg_image_resize_to_fit(VALUE, VALUE, VALUE);
static VALUE rb_epeg_image_resize_to_fill(VALUE, VALUE, VALUE);
static VALUE rb_epeg_image_crop(int, VALUE *, VALUE);
static VALUE rb_epeg_image_set_quality(VALUE, VALUE);

static VALUE rb_epeg_image_write(VALUE, VALUE);
static VALUE rb_epeg_image_to_blob(VALUE);
static VALUE rb_epeg_image_close(VALUE);
static VALUE rb_epeg_image_is_closed(VALUE);

static void rb_epeg_image_destroy(void *);
