#ifdef __cplusplus
extern "C" {
#endif

  #ifdef S7_VERSION
  bool s7extra_is_place(s7_pointer place);
  Place s7extra_place(s7_scheme *s7, s7_pointer place);
  s7_pointer s7extra_make_place(s7_scheme *radiums7_sc, Place place);

  bool s7extra_is_dyn(s7_pointer dyn);
  dyn_t s7extra_dyn(s7_scheme *s7, s7_pointer s);
  s7_pointer s7extra_make_dyn(s7_scheme *radiums7_sc, const dyn_t dyn);

  func_t *s7extra_func(s7_scheme *s7, s7_pointer func);  
  #endif

  void s7extra_callFunc_void_void(func_t *func);
  void s7extra_callFunc2_void_void(const char *funcname);

  double s7extra_callFunc_double_void(func_t *func);
  double s7extra_callFunc2_double_void(const char *funcname);

  dyn_t s7extra_callFunc_dyn_void(func_t *func);
  dyn_t s7extra_callFunc2_dyn_void(const char *funcname);

  void s7extra_callFunc_void_int_bool(func_t *func, int64_t arg1, bool arg2);
  void s7extra_callFunc2_void_int_bool(const char *funcname, int64_t arg1, bool arg2);
  
  void s7extra_callFunc_void_int(func_t *func, int64_t arg1);
  void s7extra_callFunc2_void_int(const char *funcname, int64_t arg1);

  void s7extra_callFunc_void_double(func_t *func, double arg1);
  void s7extra_callFunc2_void_double(const char *funcname, double arg1);

  void s7extra_callFunc2_void_double(const char *funcname, double arg1);
  void s7extra_callFunc_void_bool(func_t *func, bool arg1);

  void s7extra_callFunc_void_int_charpointer(func_t *func, int64_t arg1, const char* arg2);
  void s7extra_callFunc2_void_int_charpointer(const char *funcname, int64_t arg1, const char* arg2);

  void s7extra_callFunc_void_int_charpointer_bool_bool(func_t *func, int64_t arg1, const char* arg2, bool arg3, bool arg4);
  void s7extra_callFunc2_void_int_charpointer_bool_bool(const char *funcname, int64_t arg1, const char* arg2, bool arg3, bool arg4);

  void s7extra_callFunc_void_int_int(func_t *func, int64_t arg1, int64_t arg2);
  void s7extra_callFunc2_void_int_int(const char *funcname, int64_t arg1, int64_t arg2);

  void s7extra_callFunc_void_int_float_float(func_t *func, int64_t arg1, float arg2, float arg3);
  void s7extra_callFunc2_void_int_float_float(const char *funcname, int64_t arg1, float arg2, float arg3);

  void s7extra_callFunc_void_int_int_float_float(func_t *func, int64_t arg1, int64_t arg2, float arg3, float arg4);
  void s7extra_callFunc2_void_int_int_float_float(const char *funcname, int64_t arg1, int64_t arg2, float arg3, float arg4);

  bool s7extra_callFunc_bool_int_int_float_float(func_t *func, int64_t arg1, int64_t arg2, float arg3, float arg4);
  bool s7extra_callFunc2_bool_int_int_float_float(const char *funcname, int64_t arg1, int64_t arg2, float arg3, float arg4);

  int64_t s7extra_callFunc_int_int_int_int(func_t *func, int64_t arg1, int64_t arg2, int64_t arg3);
  int64_t s7extra_callFunc2_int_int_int_int(const char *funcname, int64_t arg1, int64_t arg2, int64_t arg3);

  int64_t s7extra_callFunc_int_int_int_int_bool(func_t *func, int64_t arg1, int64_t arg2, int64_t arg3, bool arg4);
  int64_t s7extra_callFunc2_int_int_int_int_bool(const char *funcname, int64_t arg1, int64_t arg2, int64_t arg3, bool arg4);

  void s7extra_callFunc_void_charpointer(func_t *func, const char* arg1);

  void s7extra_protect(void *v);
  void s7extra_unprotect(void *v);
  
#ifdef __cplusplus
}
#endif

