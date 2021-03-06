/* Copyright 2012 Kjetil S. Matheussen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#include <unistd.h>

#include <QDesktopServices>
#include <QDir>
#include <QString>
#include <QFileInfo>
#include <QLocale>
#include <QCoreApplication>


#include "../common/nsmtracker.h"

#include "helpers.h"

#include "../common/visual_proc.h"

#include "../common/OS_string_proc.h"
#include "../common/OS_settings_proc.h"


const char *OS_get_directory_separator(void){
  static char ret[2] = {0};
  static bool is_inited = false;

  if(is_inited==false){
    ret[0] = QString(QDir::separator()).toUtf8()[0];
    is_inited=true;
  }

  return ret;
}

void OS_set_argv0(char *argv0){
  /*
  //QFileInfo info(QDir::currentPath() + QString(OS_get_directory_separator()) + QString(argv0));

  QString path =  ; //info.canonicalPath();
  g_program_path = (const char*)malloc(path.size() + 10);
  sprintf((char*)g_program_path,"%s",path.toUtf8().constData());
  
  printf("current path: -%s-\n",g_program_path);
  */

  //chdir(QCoreApplication::applicationDirPath().toUtf8().constData());
  QDir::setCurrent(QCoreApplication::applicationDirPath());
}

bool OS_has_full_program_file_path(QString filename){
  QDir dir(QCoreApplication::applicationDirPath());
  QFileInfo info(dir, filename);

  if (!info.exists()){
    return false;
  }

  return true;
}

QString OS_get_full_program_file_path(QString filename){
  QDir dir(QCoreApplication::applicationDirPath());
  QFileInfo info(dir, filename);

  if (!info.exists()){
    MyQMessageBox msgBox;
    msgBox.setText("The file " + info.absoluteFilePath() + " does not exist. Make sure all files in the zip file are unpacked before starting the program. Exiting program.");
    safeExec(&msgBox);
    exit(-1);
    abort();
  }

  return QDir::toNativeSeparators(info.absoluteFilePath());
}

wchar_t *OS_get_full_program_file_path(const wchar_t *filename){
  QString ret = OS_get_full_program_file_path(STRING_get_qstring(filename));
  return STRING_create(ret);
}

// TODO: Remove.
const char *OS_get_program_path(void){
  return talloc_strdup(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()).toUtf8().constData());
}

wchar_t *STRING_create(const QString s){
  int size = (int)sizeof(wchar_t)*(s.length()+1);
  wchar_t *array = (wchar_t*)talloc_atomic(size);
  memset(array, 0, size);
  s.toWCharArray(array);
  return array;
}

wchar_t *STRING_create(const char *s){
  QString string = QString::fromUtf8(s);
  return STRING_create(string);
}

wchar_t *STRING_copy(const wchar_t *string){
  return STRING_create(STRING_get_qstring(string));
}

char* STRING_get_chars(const wchar_t *string){
  QString s = STRING_get_qstring(string);
  return talloc_strdup(s.toUtf8().constData());
}

char* STRING_get_utf8_chars(const char* s){
  QString qstring = QString::fromUtf8(s);
  return talloc_strdup(qstring.toUtf8().constData());
}

bool STRING_ends_with(const wchar_t *string, const char *endswith){
  QString s = STRING_get_qstring(string);
  return s.endsWith(endswith);
}

bool STRING_equals2(const wchar_t *s1, const wchar_t *s2){
  return STRING_get_qstring(s1) == STRING_get_qstring(s2);
}

bool STRING_equals(const wchar_t *string, const char *s2){
  QString s = STRING_get_qstring(string);
  return s == QString(s2);
}

wchar_t *STRING_replace(const wchar_t *string, const char *a, const char *b){
  QString s = STRING_get_qstring(string);
  return STRING_create(s.replace(a,b));
}

wchar_t *STRING_append(const wchar_t *s1, const wchar_t *s2){
  return STRING_create(STRING_get_qstring(s1) + STRING_get_qstring(s2));
}

wchar_t *STRING_toBase64(const wchar_t *s){
  QString s2 = STRING_get_qstring(s);
  QString encoded = s2.toLocal8Bit().toBase64();
  return STRING_create(encoded);
}

wchar_t *STRING_fromBase64(const wchar_t *encoded){
  QString encoded2 = STRING_get_qstring(encoded);
  QString decoded = QString::fromLocal8Bit(QByteArray::fromBase64(encoded2.toLocal8Bit()).data());
  return STRING_create(decoded);
}

// TODO: Rename to OS_get_program_path
const wchar_t *OS_get_program_path2(void){
  static wchar_t *array=NULL;
  if (array==NULL){
    QString s = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
    array = (wchar_t*)V_calloc(1, sizeof(wchar_t)*(s.length()+1));
    s.toWCharArray(array);
  }

  return array;
}

bool OS_config_key_is_color(const char *key){
  return QString(key).contains("color", Qt::CaseInsensitive);
}

static QDir get_dot_radium_dir(int *error){
  *error = 0;

#ifdef USE_QT5
  QString home_path = QDir::homePath();
#else
  QString home_path = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif

  {
    QFileInfo info(home_path);

    if(info.exists()==false){
#ifdef USE_QT5
      home_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
      home_path = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
    }
  }
  
  QDir dir(home_path);

  if(dir.mkpath(".radium")==false){
    GFX_Message(NULL, "Unable to create config directory");
    *error = 1;
  }

  else if(dir.cd(".radium")==false){
    GFX_Message(NULL, "Unable to read config directory\n");
    *error = 1;
  }

  return dir;
}

bool OS_has_conf_filename(QString filename){
  QString path;

  int error;
  QDir dir = get_dot_radium_dir(&error);
  if(error!=0)
    return false;

  QFileInfo info(dir, filename);

  if(info.exists()==false)
    return OS_has_full_program_file_path(filename);

  return true;
}

static QString get_custom_conf_filename(QString filename){
  int error;
  QDir dir = get_dot_radium_dir(&error);
  if (error != 0)
    return "";
  
  QFileInfo info(dir, filename);

  if(info.exists()==false)
    return "";

  return info.absoluteFilePath();
}
  
QString OS_get_conf_filename(QString filename){
  QString path;

  int error;
  QDir dir = get_dot_radium_dir(&error);
  if(error!=0)
    return "";

  QFileInfo info(dir, filename);

  if(info.exists()==false)
    info = QFileInfo(OS_get_full_program_file_path(filename));

  printf("************* conf filename: -%s\n",info.absoluteFilePath().toUtf8().constData());
  return info.absoluteFilePath();
}

char *OS_get_conf_filename2(const char *filename){
  return talloc_strdup(OS_get_conf_filename(filename).toUtf8().constData());
}

bool OS_has_conf_filename2(const char *filename){
  return OS_has_conf_filename(filename);
}

QString OS_get_keybindings_conf_filename(void){
  return OS_get_full_program_file_path("keybindings.conf");
  //return OS_get_conf_filename("keybindings.conf");
}

char *OS_get_keybindings_conf_filename2(void){
  return talloc_strdup(OS_get_keybindings_conf_filename().toUtf8().constData());
}

QString OS_get_custom_keybindings_conf_filename(void){
  int error;
  QDir dir = get_dot_radium_dir(&error);
  if(error!=0)
    return NULL;

  QFileInfo config_info(dir, "keybindings.conf");

  return config_info.absoluteFilePath();
}

char *OS_get_custom_keybindings_conf_filename2(void){
  return talloc_strdup(OS_get_custom_keybindings_conf_filename().toUtf8().constData());
}

QString OS_get_menues_conf_filename(void){
  return OS_get_conf_filename("menues.conf");
}

char *OS_get_menues_conf_filename2(void){
  return talloc_strdup(OS_get_menues_conf_filename().toUtf8().constData());
}

QString OS_get_config_filename(const char *key){
  bool is_color_config = OS_config_key_is_color(key);

  int error;
  QDir dir = get_dot_radium_dir(&error);
  if(error!=0)
    return NULL;

  QFileInfo config_info(dir, is_color_config ? "colors" : "config");

  //printf("dir: \"%s\"\n",config_info.absoluteFilePath().toUtf8().constData());

#if 0
  if(is_playing())
    abort();
#endif

  return config_info.absoluteFilePath();
}

double OS_get_double_from_string(const char *s){
  QLocale::setDefault(QLocale::C);
  QString string(s);
  return string.toDouble();
}

char *OS_get_string_from_double(double d){
  QString string = QString::number(d,'g',16);
  return talloc_strdup(string.toUtf8().constData());
}

QString OS_get_qstring_from_double(double d){
  return QString::number(d,'g',16);
}
