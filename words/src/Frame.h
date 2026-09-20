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
using MenuMap = std::map<ENUM_MENU, GtkWidget *>;

class Frame : WordsBase {
  static const int COMBOLINE_MARGIN = 3;

  GtkWidget *m_widget;
  GtkWidget *m_menu;
  GtkWidget *m_text;
  GtkWidget *m_helperUp;
  GtkWidget *m_combo[COMBOBOX_SIZE];
  GtkWidget *m_entry[ENTRY_SIZE];
  GtkWidget *m_status;
  GtkWidget *m_statusMessage;
  GtkWidget *m_searchLabel;
  GtkWidget *m_searchTagLabel;
  GtkWidget *m_searchButton[2]; // next, previous buttons
  GtkWidget *m_currentDictionary;
  GtkWidget *m_filterFrame;
  GtkWidget *m_check;
  GtkWidget *m_comboline;
  GtkWidget *m_radio;
  GtkWidget *m_textView;

  MenuMap m_menuMap;
  std::vector<GtkAccelGroup *> m_accelGroup;
  bool m_lockSignals;
  int m_tagIndex;
  int m_tags;
  std::jthread m_thread;
  CheckNewVersion m_newVersion;
  guint m_debounce_timer_id = 0;
  gint getComboIndex(ENUM_COMBOBOX e) const {
    assert(e != COMBOBOX_SIZE);
    assert(GTK_IS_COMBO_BOX(m_combo[e]));
    return gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[e]));
  }

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

  void addEntryForTemplate();
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
  GtkWidget *createLabel(ENUM_STRING e) { return createLabel(m_language[e]); }

  /*make non static because other add() functions are not static*/
  void add(GtkWidget *w, GtkWidget *a) {
    gtk_box_pack_start(GTK_BOX(w), a, TRUE, TRUE, 0);
  }

  void add(GtkWidget *w, std::string s) { add(w, createLabel(s)); }
  void add(GtkWidget *w, ENUM_STRING e) { add(w, createLabel(e)); }

public:
  Frame();

  void destroy();

  void startJob(bool clearResult);
  void endJob();
  bool prepare(); // return true if entry data is valid

  void routine();
  void sortFilterAndUpdateResults();

  bool isSignalsLocked() { return m_lockSignals; }

  void lockSignals() { m_lockSignals = true; }

  void unlockSignals() { m_lockSignals = false; }

  void clickMenu(ENUM_MENU menu);

  void setDictionary();
  void loadAndUpdateCurrentLanguage();
  void comboChanged(ENUM_COMBOBOX e);
  void radioChanged(GtkWidget *w);
  void clickButton(GtkWidget *button);

  virtual bool userBreakThread();
  virtual void setMenuLabel(ENUM_MENU e, std::string const &text) override;
  virtual void endJobThread() override;
  std::string getMenuLabel(ENUM_MENU e);

  void stopThreadAndNewRoutine();
  void stopThread();
  // void waitThread();
  void startThread(bool full);//false - only sort/filter

  void updateTextView() { updateTextView(m_text, m_out); }

  void updateTextView(GtkWidget *view, std::string const &s);

  void setStatus(std::string const &s);

  void connectEntrySignals(ENTRY_ENUM e);
  void entryFocusChanged(bool in);
  void removeAccelerators();
  void addAccelerators();

  void sortOrFilterChanged();

  void refillCombo(ENUM_COMBOBOX e, ENUM_STRING first, int length);
  void newVersionMessage();

  void setDebounceTimer(ENTRY_ENUM e);
  void debounceTimeout(ENTRY_ENUM e);

  void setLabel(GtkWidget *w, ENUM_STRING e);
  void setLabel(GtkWidget *w, const std::string &s);
  bool setCheckFilterRegex();
};
