/*
 * Frame.h
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#pragma once

#include "CheckNewVersion.h"
#include "common/WordsBase.h"
#include "common/consts.h"
#include <initializer_list>

using MenuMap = std::map<ENUM_MENU, GtkWidget *>;
using WB = std::initializer_list<std::pair<GtkWidget *, bool>>;

class Frame : WordsBase {
  static const int COMBOLINE_MARGIN = 3;

  GtkWidget *m_widget;
  GtkWidget *m_menu;
  GtkWidget *m_text[TEXTVIEW_SIZE];
  GtkWidget *m_helperUp;
  GtkWidget *m_combo[COMBOBOX_SIZE];
  GtkWidget *m_entry[ENTRY_SIZE];
  GtkWidget *m_status;
  GtkWidget *m_statusMessage;
  GtkWidget *m_searchTagLabel;
  GtkWidget *m_button[BUTTON_SIZE];
  GtkWidget *m_currentDictionary;
  GtkWidget *m_check;
  GtkWidget *m_comboline;
  GtkWidget *m_radio;

  MenuMap m_menuMap;
  std::vector<GtkAccelGroup *> m_accelGroup;
  bool m_lockSignals;
  std::jthread m_thread;
  CheckNewVersion m_newVersion;
  guint m_debounce_timer_id = 0;

  int m_tagIndex = 0;
  int m_tags = 0;
  struct TagRange {
    GtkTextMark *start_mark;
    GtkTextMark *end_mark;
  };
  std::vector<TagRange> m_found_tags;
  void clearTagMarks();

  gint getComboIndex(ENUM_COMBOBOX e) const;
  void setComboIndex(ENUM_COMBOBOX e, gint v);

  void updateComboValue(ENUM_COMBOBOX e);

  void aboutDialog();
  void setHelperPanel();

  void createImageCombo(ENUM_COMBOBOX e);
  GtkWidget *createTextCombo(ENUM_COMBOBOX e, VString v, int active);
  GtkWidget *createTextCombo(ENUM_COMBOBOX e); // empty
  GtkWidget *createTextCombo(ENUM_COMBOBOX e, int from, int to, int active);
  GtkWidget *createTextCombo(ENUM_COMBOBOX e, ENUM_STRING from, ENUM_STRING to,
                             int active);

  void addComboLineToHelper(ENUM_STRING id, int from, int to, int active,
                            ENUM_STRING eid, bool any = false);
  void addComboLineToHelper(ENUM_STRING id, int from, int to, int active,
                            std::string s1, std::string s2, std::string s3,
                            bool any = false);
  void addComboToHelper(ENUM_STRING from, ENUM_STRING to, int active,
                        ENUM_COMBOBOX comboboxId = COMBOBOX_HELPER0);

  void updateTags(int n);

  /*make non static because other createLabel() functions is not static*/
  GtkWidget *createLabel(std::string s) { return gtk_label_new(s.c_str()); }
  GtkWidget *createLabel(ENUM_STRING e) { return createLabel(string(e)); }

  /*make non static because other add() functions are not static*/
  static void add(GtkWidget *w, GtkWidget *a, bool b = true) {
    gtk_box_pack_start(GTK_BOX(w), a, b, b, 0);
  }

  void add(GtkWidget *w, std::string s) { add(w, createLabel(s)); }
  void add(GtkWidget *w, ENUM_STRING e) { add(w, createLabel(e)); }
  static GtkWidget *createBox(GtkOrientation o,int margin, WB wb);

public:
  Frame();

  void destroy();

  void endJob();

  void routine(bool full = true);

  bool isSignalsLocked() { return m_lockSignals; }

  void lockSignals() { m_lockSignals = true; }

  void unlockSignals() { m_lockSignals = false; }

  void clickMenu(ENUM_MENU menu);

  void updateDictionary();
  void loadAndUpdateCurrentLanguage();
  void comboChanged(ENUM_COMBOBOX e);
  void radioChanged(GtkWidget *w);
  void clickButton(GtkWidget *button);

  virtual void setMenuLabel(ENUM_MENU e, std::string const &text) override;
  virtual void endJobThread() override;
  std::string getMenuLabel(ENUM_MENU e);

  void stopThreadAndNewRoutine(bool full = true);
  void stopThread();
  void startThread(bool full); // false - only sort/filter

  void updateTextView(ENUM_TEXTVIEW e, std::string const &s);

  void connectEntrySignals(ENUM_ENTRY e);
  void entryFocusChanged(bool in);
  void removeAccelerators();
  void addAccelerators();

  void refillCombo(ENUM_COMBOBOX e, ENUM_STRING first, int length);
  void newVersionMessage();

  void setDebounceTimer(ENUM_ENTRY e);
  void debounceTimeout(ENUM_ENTRY e);

  void setLabel(GtkWidget *w, ENUM_STRING e);
  void setLabel(GtkWidget *w, const std::string &s);

  virtual std::string getEntryString(ENUM_ENTRY e) const override;
  virtual std::string getTextViewString() const override;
  virtual bool getCheck() const override;

  void entryChanged(ENUM_ENTRY e);

  GtkTextBuffer *tvBuffer(ENUM_TEXTVIEW e = TEXTVIEW_MAIN) const;
  std::string getProgramVersionString() const;
  void updateStatus();
  void setSensitiveOrderFilter(bool b);
  void setPlaceholder(ENUM_ENTRY e, ENUM_STRING s);
  void updateButton(ENUM_BUTTON e);

  StartStopButtonState getStartStopState() const;
};
