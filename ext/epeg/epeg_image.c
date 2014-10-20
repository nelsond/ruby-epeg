#include <epeg_image.h>

void Init_epeg()
{
  VALUE mEpeg      = rb_define_module("Epeg"),
        cEpegImage = rb_define_class_under(mEpeg, "Image", rb_cObject);

  rb_cv_set(cEpegImage, "@@quality", INT2NUM(85));

  rb_define_singleton_method(cEpegImage, "from_blob",        rb_epeg_image_from_blob,           1);
  rb_define_singleton_method(cEpegImage, "open",             rb_epeg_image_open,                1);
  rb_define_singleton_method(cEpegImage, "default_quality=", rb_epeg_image_set_default_quality, 1);
  rb_define_singleton_method(cEpegImage, "default_quality",  rb_epeg_image_get_default_quality, 0);

  rb_define_method(cEpegImage, "initialize", rb_epeg_image_initialize,  0);

  rb_define_method(cEpegImage, "resize",         rb_epeg_image_resize,              2);
  rb_define_method(cEpegImage, "resize_to_fit",  rb_epeg_image_resize_to_fit,       2);
  rb_define_method(cEpegImage, "resize_to_fill", rb_epeg_image_resize_to_fill,      2);
  rb_define_method(cEpegImage, "crop",           rb_epeg_image_crop,               -1);
  rb_define_method(cEpegImage, "write",          rb_epeg_image_write,               1);
  rb_define_method(cEpegImage, "to_blob",        rb_epeg_image_to_blob,             0);
  rb_define_method(cEpegImage, "close",          rb_epeg_image_close,               0);
  rb_define_method(cEpegImage, "closed?",        rb_epeg_image_is_closed,           0);
  rb_define_method(cEpegImage, "quality",        rb_epeg_image_get_or_set_quality, -1);
  rb_define_method(cEpegImage, "quality=",       rb_epeg_image_set_quality,         1);

  rb_define_attr(cEpegImage, "width",   1, 0);
  rb_define_attr(cEpegImage, "height",  1, 0);
}

/*
 * call-seq:
 *  open(file_path)
 *
 * Creates a new Image from +file_path+
 *
 *     img = Epeg::Image.open("./example.jpg")
 */
static VALUE rb_epeg_image_open(VALUE klass, VALUE file_path)
{
  Check_Type(file_path, T_STRING);

  Epeg_Image *image = epeg_file_open(StringValueCStr(file_path));

  if(!image) {
    rb_raise(rb_eRuntimeError, "Error: unable to read file \"%s\"\n",
                               StringValueCStr(file_path));
  }

  VALUE image_obj = Data_Wrap_Struct(klass, NULL, rb_epeg_image_destroy, image);
  rb_obj_call_init(image_obj, 0, NULL);

  return image_obj;
}

static VALUE rb_epeg_image_from_blob(VALUE klass, VALUE blob)
{
  Epeg_Image *image = epeg_memory_open(RSTRING_PTR(blob), RSTRING_LEN(blob));

  if(!image) { rb_raise(rb_eRuntimeError, "Error: unable to read blob"); }

  VALUE image_obj = Data_Wrap_Struct(klass, NULL, rb_epeg_image_destroy, image);

  rb_obj_call_init(image_obj, 0, NULL);

  return image_obj;
}

/*
 * call-seq:
 *  default_quality=(q)
 *
 * Sets the default quality (+q+ >= 0 and +q+ <= 100)
 *
 *     Epeg::Image.default_quality = 10
 */
static VALUE rb_epeg_image_set_default_quality(VALUE klass, VALUE q)
{
  Check_Type(q, T_FIXNUM);
  int quality = NUM2INT(q);

  rb_cv_set(klass, "@@quality", q);

  return Qnil;
}

/*
 * call-seq:
 *  default_quality
 *
 * Returns the current default quality for all images.
 *
 *     Epeg::Image.default_quality #=> 85
 */
static VALUE rb_epeg_image_get_default_quality(VALUE klass)
{
  return rb_cv_get(klass, "@@quality");
}

/*
 * call-seq:
 *  intialize()
 *
 * See Epeg::Image.open
 */
static VALUE rb_epeg_image_initialize(VALUE self)
{
  Epeg_Image *image;
  Data_Get_Struct(self, Epeg_Image, image);

  VALUE q = rb_cv_get(CLASS_OF(self), "@@quality");
  epeg_quality_set(image, NUM2UINT(q));
  rb_iv_set(self, "@quality", q);

  epeg_comment_set(image, (char *)NULL);

  int w, h;
  epeg_size_get(image, &w, &h);

  rb_iv_set(self, "@width",     INT2NUM(w));
  rb_iv_set(self, "@height",    INT2NUM(h));

  rb_iv_set(self, "epeg_file_closed", Qfalse);
  rb_iv_set(self, "epeg_trimmed",     Qfalse);

  return self;
}

/*
 * call-seq:
 *  resize(w, h)
 *
 * Resizes image to +w+ x +h+. Does not keep aspect ratio of the image
 *
 *     img = Epeg::Image.open("./example.jpg")
 *     img.resize(width, height)
 */
static VALUE rb_epeg_image_resize(VALUE self, VALUE w, VALUE h)
{
  Check_Type(w, T_FIXNUM);
  Check_Type(h, T_FIXNUM);

  Epeg_Image *image;
  Data_Get_Struct(self, Epeg_Image, image);

  epeg_decode_bounds_set(image, 0, 0, NUM2UINT(w), NUM2UINT(h));
  rb_iv_set(self, "epeg_trimmed", Qfalse);

  return self;
}

/*
 * call-seq:
 *  resize_to_fit(w, h)
 *
 * Resizes image to fit +w+ x +h+. Please not that the actual width or
 * height of the image can be smaller than +w+ respectively +h+.
 * See Epeg::Image.resize_to_fill
 */
static VALUE rb_epeg_image_resize_to_fit(VALUE self, VALUE w, VALUE h)
{
  Check_Type(w, T_FIXNUM);
  Check_Type(h, T_FIXNUM);

  const double image_width   = (double)NUM2INT(rb_iv_get(self, "@width"));
  const double image_height  = (double)NUM2INT(rb_iv_get(self, "@height"));

  const double fit_width     = (double)NUM2INT(w);
  const double fit_height    = (double)NUM2INT(h);

  const double width_ratio   = image_width/fit_width;
  const double height_ratio  = image_height/fit_height;

  float width, height;

  if(width_ratio == height_ratio) {
    width  = fit_width;
    height = fit_width;
  } else {
    if(width_ratio > height_ratio) {
      width  = fit_width;
      height = fit_width*(image_height/image_width);
    } else {
      width  = fit_height*(image_width/image_height);
      height = fit_height;
    }
  }

  return rb_epeg_image_resize(  self, INT2NUM( ceil(width) ), INT2NUM( ceil(height) )  );
}

/*
 * call-seq:
 *  resize_to_fill(w, h)
 *
 * Resizes image to fill +w+ x +h+. Please not that the actual width or
 * height of the image can be larger than +w+ respectively +h+.
 * See Epeg::Image.resize_to_fit
 */
static VALUE rb_epeg_image_resize_to_fill(VALUE self, VALUE w, VALUE h)
{
  Check_Type(w, T_FIXNUM);
  Check_Type(h, T_FIXNUM);

  const double image_width   = (double)NUM2INT(rb_iv_get(self, "@width"));
  const double image_height  = (double)NUM2INT(rb_iv_get(self, "@height"));

  const double fit_width     = (double)NUM2INT(w);
  const double fit_height    = (double)NUM2INT(h);

  const double width_ratio   = image_width/fit_width;
  const double height_ratio  = image_height/fit_height;

  float width, height;

  if(width_ratio == height_ratio) {
    width  = fit_width;
    height = fit_width;
  } else {
    if(width_ratio > height_ratio) {
      width  = fit_height*(image_width/image_height);
      height = fit_height;
    } else {
      width  = fit_width;
      height = fit_width*(image_height/image_width);
    }
  }

  return rb_epeg_image_resize(  self, INT2NUM( ceil(width) ), INT2NUM( ceil(height) )  );
}

/*
 * call-seq:
 *  cropp(w, h, [x, y])
 *
 * Crops image to +w+ x +h+. If +x+ and +y+ are not specified the image is
 * cropped with the center gravity.
 */
static VALUE rb_epeg_image_crop(int argc, VALUE *argv, VALUE self)
{
  if(argc < 2 || argc == 3 || argc > 4) {
    rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or 4)", argc);
  }

  Check_Type(argv[0], T_FIXNUM);
  Check_Type(argv[1], T_FIXNUM);

  unsigned int w = NUM2UINT(argv[0]);
  unsigned int h = NUM2UINT(argv[1]);

  unsigned int iw = NUM2UINT(rb_iv_get(self, "@width"));
  unsigned int ih = NUM2UINT(rb_iv_get(self, "@height"));

  unsigned int x, y;

  if(w > iw){ w = iw; }
  if(h > ih){ h = ih; }

  // crop with gravity = center
  if(argc == 2) {
    x = (int)(  ceil( ((double)iw - (double)w)/2 )  );
    y = (int)(  ceil( ((double)ih - (double)h)/2 )  );
  }

  // crop with origin at (x,y)
  if (argc == 4) {
    Check_Type(argv[2], T_FIXNUM);
    Check_Type(argv[3], T_FIXNUM);

    x = NUM2INT(argv[2]);
    y = NUM2INT(argv[3]);
  }

  if (x >= iw) { x = iw - 1; }
  if (y >= ih) { y = ih - 1; }

  if (w + x >= iw) { w = iw - x; }
  if (h + y >= ih) { h = ih - y; }

  // FIXME: epeg doesn't seem to be able to set bounds to (0,0,iw,ih)
  //        and encode/write the image to memory or file
  if (w == iw) { w--; }
  if (h == ih) { h--; }

  Epeg_Image *image;
  Data_Get_Struct(self, Epeg_Image, image);

  epeg_decode_bounds_set(image, x, y, w, h);
  rb_iv_set(self, "epeg_trimmed", Qtrue);

  return self;
}

/*
 * internal use
 */
static void rb_epeg_image_encode_or_trim(VALUE obj, Epeg_Image *image) {
  int status;

  if(rb_iv_get(obj, "epeg_trimmed") == Qtrue) {
    status = epeg_trim(image);
  } else {
    status = epeg_encode(image);
  }

  if(status != 0) {  rb_raise(rb_eRuntimeError, "Error: can't encode"); }
}

/*
 * call-seq:
 *  write(file_path)
 *
 * Writes the image to +file_path+ and closes the image stream.
 * Please note that you can't do any further operations on the image
 * after using this method.
 */
static VALUE rb_epeg_image_write(VALUE self, VALUE file_path)
{
  Check_Type(file_path, T_STRING);

  if(rb_iv_get(self, "epeg_file_closed") != Qfalse) {
    rb_raise(rb_eRuntimeError, "Error: closed file");
  }

  Epeg_Image *image;
  Data_Get_Struct(self, Epeg_Image, image);

  epeg_file_output_set(image, StringValueCStr(file_path));
  rb_epeg_image_encode_or_trim(self, image);

  rb_epeg_image_close(self);

  return Qnil;
}

/*
 * call-seq:
 *  to_blob()
 *
 * Returns the image data as blob and closes the image stream.
 * Please note that you can't do any further operations on the image
 * after using this method.
 */
static VALUE rb_epeg_image_to_blob(VALUE self)
{
  if(rb_iv_get(self, "epeg_file_closed") != Qfalse) {
    rb_raise(rb_eRuntimeError, "Error: closed file");
  }

  Epeg_Image *image;
  Data_Get_Struct(self, Epeg_Image, image);

  char *data;
  int size;

  epeg_memory_output_set(image, &data, &size);
  rb_epeg_image_encode_or_trim(self, image);

  VALUE blob = rb_str_new(data, size);
  free(data);

  rb_epeg_image_close(self);

  return blob;
}

/*
 * call-seq:
 *  quality((q))
 *
 * Returns the image quality or sets it if +q+ is given.
 * See Epeg::Image#quality=
 */
static VALUE rb_epeg_image_get_or_set_quality(int argc, VALUE *argv, VALUE self)
{
  if(argc > 1) {
    rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
  }

  if(argc == 0) { return rb_iv_get(self, "@quality"); }

  if(argc == 1) {
    rb_epeg_image_set_quality(self, argv[0]);
    return self;
  }

}

/*
 * call-seq:
 *  quality=(q)
 *
 * Sets the quality of the image to +q+ (+q+ >= 0 and +q+ <= 100).
 * See Epeg::Image#quality
 */
static VALUE rb_epeg_image_set_quality(VALUE self, VALUE q)
{
  Check_Type(q, T_FIXNUM);

  int quality = NUM2INT(q);

  if (quality < 0)   { rb_raise(rb_eRuntimeError, "Error: quality must be >= 0"); }
  if (quality > 100) { rb_raise(rb_eRuntimeError, "Error: quality must be <= 100"); }

  Epeg_Image *image;
  Data_Get_Struct(self, Epeg_Image, image);

  epeg_quality_set(image, quality);
  rb_iv_set(self, "@quality", INT2NUM(quality));

  return Qnil;
}

/*
 * call-seq:
 *  close()
 *
 * Closes the image stream.
 * Please note that you can't do any furhter operations on the image
 * after using this method.    See Epeg::Image#closed?
 */
static VALUE rb_epeg_image_close(VALUE self)
{
  if(rb_iv_get(self, "epeg_file_closed") == Qfalse) {
    Epeg_Image *image;
    Data_Get_Struct(self, Epeg_Image, image);

    epeg_close(image);
    rb_iv_set(self, "epeg_file_closed", Qtrue);
  }

  return Qnil;
}

/*
 * call-seq:
 *  closed?()
 *
 * Returns true if the image stream is already closed.
 * See Epeg::Image#close
 */
static VALUE rb_epeg_image_is_closed(VALUE self)
{
  return rb_iv_get(self, "epeg_file_closed");
}

static void rb_epeg_image_destroy(void *image)
{
  if(!image) { epeg_close((Epeg_Image *)image); }
}
