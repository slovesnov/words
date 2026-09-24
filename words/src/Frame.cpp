/*
 * Frame.cpp
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

/*
 * WINDOW_SIZE_TYPE=0 default
 * WINDOW_SIZE_TYPE=1 for youtube helper clip writing
 * WINDOW_SIZE_TYPE=2 for site screenshots
 */
#define WINDOW_SIZE_TYPE 0

#include "Frame.h"
#include "CheckNewVersion.h"
#if WINDOW_SIZE_TYPE == 2
#include <windows.h>
#endif

#include "magic_enum.hpp" //TODO
#include <format>
#include <unordered_map>

// Note WORDS_VERSION defined in consts.h
const char MAIL[] = "slovesnov@yandex.ru";
const std::string URL = "https://slovesnov.rf.gd/";
const std::string HOMEPAGE = URL + "?words";
const std::string HOMEPAGE_ONLINE = URL + "?words_online";
const char markTag[] = "mark";
const char activeTag[] = "active";
const char CERROR[] = "cerror";
const int TIMER = 400; // milliseconds
const int MIN_LEFT_PANEL_WIDTH = 800;
const int MIN_RIGHT_PANEL_WIDTH = 420;
const std::string CONFIG_TAGS[] = {"version", "language", "dictionary",
                                   "separator"};
const char DOWNLOAD_URL[] =
    "http://sourceforge.net/projects/javawords/files/latest/download";
extern std::string LNG[LANGUAGES];

/* too many items in combobox, so set maximum bound.
  constants calculated in functions
  showLongestAnagram();
  showLongestPangram();
  showLongestSimpleWordSequence();
  showLongestDoubleWordSequence();
  onchange dictionay size need to recount
  also on loading need to set dictionary size for vector, if use vector
 */
const int MAX_ANAGRAM_LENGTH = 31;              //{22 31}
const int MAX_PANGRAM_LENGTH = 21;              //{16 21}
const int MAX_SIMPLE_WORD_SEQUENCE_LENGTH = 30; //{25 30}
const int MAX_DOUBLE_WORD_SEQUENCE_LENGTH = 14; //{7 14}

const std::unordered_map<ENUM_MENU, ENUM_STRING> MENU_TO_HELP_STRING = {
    {MENU_ANAGRAM, ANAGRAM_HELP},
    {MENU_PANGRAM, PANGRAM_HELP},
    {MENU_TEMPLATE, TEMPLATE_HELP},
    {MENU_PALINDROME, PALINDROME_HELP},
    {MENU_CROSSWORD, CROSSWORD_HELP},
    {MENU_REGULAR_EXPRESSIONS, REGULAR_EXPRESSION_HELP},
    {MENU_MODIFICATION, MODIFICATION_HELP},
    {MENU_CHAIN, CHAIN_HELP},
    {MENU_CHARACTER_SEQUENCE, CHARACTERS_SEQUENCE_HELP},
    {MENU_LETTER_GROUP_SPLIT, LETTER_GROUP_SPLIT_HELP},
    {MENU_SIMPLE_WORD_SEQUENCE, WORD_SEQUENCE_HELP},
    {MENU_DOUBLE_WORD_SEQUENCE, DOUBLE_WORD_SEQUENCE_HELP},
    {MENU_WORD_SEQUENCE_FULL, WORD_SEQUENCE_FULL_HELP},
    {MENU_KEYBOARD_WORD_SIMPLE, KEYBOARD_WORD_SIMPLE_HELP},
    {MENU_KEYBOARD_WORD_COMPLEX, KEYBOARD_WORD_DIAGONAL_HELP},
    {MENU_CONSONANT_VOWEL_SEQUENCE, CONSONANT_VOWEL_CHARACTER_SEQUENCE_HELP},
    {MENU_DENSITY, DENSITY_HELP},
    {MENU_TWO_DICTIONARIES_SIMPLE, TWO_DICTIONARIES_SIMPLE_HELP},
    {MENU_TWO_DICTIONARIES_TRANSLIT, TWO_DICTIONARIES_TRANSLIT_HELP},
    {MENU_TWO_DICTIONARIES_KEYBOARD_WORD, TWO_DICTIONARIES_KEYBOARD_WORD_HELP},
    {MENU_TWO_CHARACTERS_DISTRIBUTION, TWO_CHARACTERS_DISTRIBUTION_HELP},
    {MENU_TWO_CHARACTERS_DISTRIBUTION_START, TWO_CHARACTERS_DISTRIBUTION_HELP},
    {MENU_TWO_CHARACTERS_DISTRIBUTION_END, TWO_CHARACTERS_DISTRIBUTION_HELP}};

const std::unordered_map<ENUM_MENU, std::string> MENU_TO_ICON_FILE = {
    {MENU_SEARCH, "search.png"},
    {MENU_EDIT, "edit.png"},
    {MENU_ADDITIONS, "add.png"},
    {MENU_LANGUAGE, "language.png"},
    {MENU_HELP, "help.png"},
    {MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD, "select_all_copy.png"},
    {MENU_EDIT_SELECT_ALL, "select_all.png"},
    {MENU_EDIT_COPY_TO_CLIPBOARD, "copy.png"},
    {MENU_LOAD_ENGLISH_DICTIONARY, "en.gif"},
    {MENU_ENGLISH_LANGUAGE, "en.gif"},
    {MENU_LOAD_RUSSIAN_DICTIONARY, "ru.gif"},
    {MENU_RUSSIAN_LANGUAGE, "ru.gif"},
    {MENU_ABOUT, "word16.png"},
    {MENU_HOMEPAGE, "web.png"}};

const std::unordered_map<ENUM_MENU, int> MENU_TO_ACCEL_KEY = {
    {MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD, GDK_KEY_B},
    {MENU_EDIT_SELECT_ALL, GDK_KEY_A},
    {MENU_EDIT_COPY_TO_CLIPBOARD, GDK_KEY_C}};
Frame *frame;

gboolean end_job(gpointer) {
  frame->endJob();
  return G_SOURCE_REMOVE;
}

void menu_activate(GtkWidget *widget, ENUM_MENU menu) {
  frame->clickMenu(menu);
}

void combo_changed(GtkComboBox *comboBox, ENUM_COMBOBOX e) {
  if (!frame->isSignalsLocked()) {
    frame->comboChanged(e);
  }
}

void entry_insert(GtkWidget *entry, gchar *new_text, gint new_text_length,
                  gpointer position, ENUM_ENTRY e) {
  frame->entryChanged(e);
}

void entry_delete(GtkWidget *entry, gint start_pos, gint end_pos,
                  ENUM_ENTRY e) {
  frame->entryChanged(e);
}

gboolean entry_focus_in(GtkWidget *widget, GdkEvent *event, gpointer) {
  frame->entryFocusChanged(true);
  return TRUE;
}

gboolean entry_focus_out(GtkWidget *widget, GdkEvent *event, gpointer) {
  frame->entryFocusChanged(false);
  return TRUE;
}

void button_clicked(GtkWidget *button, gpointer) { frame->clickButton(button); }

gboolean label_clicked(GtkWidget *label, const gchar *uri, gpointer) {
  openURL(uri);
  return TRUE;
}

void check_changed(GtkWidget *check, gpointer) {
  frame->stopThreadAndNewRoutine();
}

void radio_changed(GtkWidget *radio, gpointer) { frame->radioChanged(radio); }

void text_view_changed(GtkTextBuffer *buffer, gpointer) {
  // proceed same as template entry changed
  frame->setDebounceTimer(ENTRY_TEMPLATE);
}

void destroy_window(GtkWidget *object, gpointer) { frame->destroy(); }

gboolean on_debounce_timeout(gpointer user_data) {
  frame->debounceTimeout(ENUM_ENTRY(GPOINTER_TO_INT(user_data)));
  return G_SOURCE_REMOVE;
}

gboolean new_version_message(gpointer) {
  frame->newVersionMessage();
  return G_SOURCE_REMOVE;
}

Frame::Frame() : WordsBase() {
  GtkWidget *w, *w1, *w2, *scroll;
  GtkWidget *item;
  bool bSubMenu;
  int i, j;
  std::vector<GtkMenuItem *> subMenu;
  std::string s;
  m_newVersion.start(WORDS_VERSION, new_version_message);
  frame = this;
  m_menuClick = MENU_SEARCH;
  // set dot as decimal separator, standard locale
  setlocale(LC_NUMERIC, "C"); // needs double to string when output time
  m_lockSignals = false;

  // load configuration file
  m_languageIndex = m_dictionaryIndex = getSystemLanguage() == "ru";
  GdkDisplay *display = gdk_display_get_default();
  GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
  GdkRectangle geometry;
  gdk_monitor_get_geometry(monitor, &geometry);
  m_separatorPosition = geometry.width - MIN_RIGHT_PANEL_WIDTH;
  readConfig(CONFIG_TAGS, s, m_languageIndex, m_dictionaryIndex,
             m_separatorPosition);

  m_widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  m_menu = gtk_menu_bar_new();
  m_text[TEXTVIEW_MAIN] = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(m_text[TEXTVIEW_MAIN]), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_text[TEXTVIEW_MAIN]), FALSE);
  gtk_text_buffer_create_tag(tvBuffer(), markTag, "background", "lightblue",
                             NULL);
  gtk_text_buffer_create_tag(tvBuffer(), activeTag, "background", "Khaki",
                             NULL);
  const int TEXT_VIEW_MARGIN = 5;
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(m_text[TEXTVIEW_MAIN]),
                                TEXT_VIEW_MARGIN); // TODO
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(m_text[TEXTVIEW_MAIN]),
                                 TEXT_VIEW_MARGIN);
  scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scroll), m_text[TEXTVIEW_MAIN]);

  m_helperUp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
  m_status = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
  m_statusMessage = gtk_label_new("");

  createImageCombo(COMBOBOX_SORT_ORDER);
  createTextCombo(COMBOBOX_SORT);
  setComboIndex(COMBOBOX_SORT_ORDER, 1);
  createTextCombo(COMBOBOX_FILTER);

  m_entry[ENTRY_SEARCH] = gtk_entry_new();
  m_searchTagLabel = gtk_label_new("");
  gtk_widget_set_size_request(m_searchTagLabel, 40, -1);

  std::string im[] = {"down.png", "up.png", "", ""};
  i = -1;
  for (auto &a : m_button) {
    i++;
    a = gtk_button_new();
    if (oneOf(i, BUTTON_NEXT, BUTTON_PREVIOUS))
      gtk_button_set_image(GTK_BUTTON(a), image(im[i]));
  }
  updateButton(BUTTON_DICTIONARY);

  m_currentDictionary = gtk_label_new("");
  m_entry[ENTRY_FILTER] = gtk_entry_new();

  for (i = 0; i < int(MENU_TO_ACCEL_KEY.size()); i++) {
    m_accelGroup.push_back(gtk_accel_group_new());
  }
  addAccelerators();

  // fill containers
  const int margin = 4;
  add(m_status, m_statusMessage);
  gtk_widget_set_halign(m_statusMessage, GTK_ALIGN_START);
  gtk_widget_set_margin_start(
      GTK_WIDGET(m_statusMessage),
      TEXT_VIEW_MARGIN); // TODO add margin for nice view

  // sort combo has many items so place it into the middle
  WB A[] = {
      {{m_button[BUTTON_STARTSTOP], false},
       {m_currentDictionary, false},
       {m_button[BUTTON_DICTIONARY], false}},

      {{m_combo[COMBOBOX_SORT], true}, {m_combo[COMBOBOX_SORT_ORDER], false}},

      {{m_entry[ENTRY_FILTER], true}, {m_combo[COMBOBOX_FILTER], false}},

      {{m_entry[ENTRY_SEARCH], true},
       {m_searchTagLabel, false},
       {m_button[BUTTON_NEXT], false},
       {m_button[BUTTON_PREVIOUS], false}}};

  w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
  for (auto &a : A) {
    w1 = createBox(GTK_ORIENTATION_HORIZONTAL, margin, a);
    gtk_container_add(GTK_CONTAINER(w), w1);
  }

  w1 = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  // left part
  w2 = createBox(GTK_ORIENTATION_VERTICAL, 0,
                 {{scroll, true}, {m_status, false}});
  gtk_widget_set_size_request(w2, MIN_LEFT_PANEL_WIDTH, -1);
  gtk_paned_pack1(GTK_PANED(w1), w2, FALSE, FALSE);

  // right part
  w2 = createBox(GTK_ORIENTATION_VERTICAL, 3,
                 {{m_helperUp, false},
                  {gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0), true},
                  {w, false}});
  gtk_widget_set_size_request(w2, MIN_RIGHT_PANEL_WIDTH, -1);
  gtk_paned_pack2(GTK_PANED(w1), w2, TRUE, FALSE);
  gtk_widget_set_margin_start(GTK_WIDGET(w2), 5);
  gtk_widget_set_margin_end(GTK_WIDGET(w2), 5);

  m_panedWidget = w1;

  m_positionSignalId = g_signal_connect(
      w1, "notify::position",
      G_CALLBACK(+[](GObject *object, GParamSpec *pspec, gpointer data) {
        frame->m_separatorPosition = gtk_paned_get_position(GTK_PANED(object));
      }),
      NULL);

  g_signal_handler_block(w1, m_positionSignalId);
  g_timeout_add(50, G_SOURCE_FUNC(+[](gpointer data) -> gboolean {
                  GtkPaned *paned = GTK_PANED(frame->m_panedWidget);
                  gtk_paned_set_position(paned, frame->m_separatorPosition);
                  g_signal_handler_unblock(paned, frame->m_positionSignalId);
                  return G_SOURCE_REMOVE;
                }),
                NULL);

  w = createBox(GTK_ORIENTATION_VERTICAL, 0, {{m_menu, false}, {w1, true}});
  gtk_container_add(GTK_CONTAINER(m_widget), w);

  // load menu
  i = 0;
  for (auto &s : readFile(m_languageIndex, "language")) {
    if (subMenu.empty() && s.empty()) {
      break;
    }

    if (s.starts_with(SEPARATOR)) {
      gtk_menu_shell_append(
          GTK_MENU_SHELL(gtk_menu_item_get_submenu(subMenu.back())),
          gtk_separator_menu_item_new());
      continue;
    }

    if (s.find('}') != std::string::npos) {
      subMenu.pop_back();
      continue;
    }

    bSubMenu = s.find('{') != std::string::npos;

    auto it = MENU_TO_ICON_FILE.find(ENUM_MENU(i));
    if (it == MENU_TO_ICON_FILE.end()) {
      item = gtk_menu_item_new_with_label("");
    } else {
      item = gtk_menu_item_new();
      w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
      w1 = gtk_accel_label_new("");
      gtk_container_add(GTK_CONTAINER(w), image(it->second));
      gtk_label_set_use_underline(GTK_LABEL(w1), TRUE);
      gtk_label_set_xalign(GTK_LABEL(w1), 0.0);
      gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(w1), item);
      gtk_box_pack_end(GTK_BOX(w), w1, TRUE, TRUE, 0);
      gtk_container_add(GTK_CONTAINER(item), w);
    }

    gtk_menu_shell_append(
        GTK_MENU_SHELL(subMenu.size() == 0 && bSubMenu
                           ? m_menu
                           : gtk_menu_item_get_submenu(subMenu.back())),
        item);

    auto it1 = MENU_TO_ACCEL_KEY.find(ENUM_MENU(i));
    if (it1 != MENU_TO_ACCEL_KEY.end()) {
      j = std::distance(MENU_TO_ACCEL_KEY.begin(), it1);
      gtk_widget_add_accelerator(item, "activate", m_accelGroup[j], it1->second,
                                 GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    }

    m_menuMap[ENUM_MENU(i)] = item;
    if (bSubMenu) {
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), gtk_menu_new());
      subMenu.push_back(GTK_MENU_ITEM(item));
    } else {
      g_signal_connect(item, "activate", G_CALLBACK(menu_activate),
                       GINT_TO_POINTER(i));
    }

    i++;
  }

  loadAndUpdateCurrentLanguage();
  // update menu enables/disables, after language[] is filled
  updateDictionary();

  loadCSS();

  connectEntrySignals(ENTRY_SEARCH);
  connectEntrySignals(ENTRY_FILTER);

  for (auto &a : m_button) {
    g_signal_connect(a, "clicked", G_CALLBACK(button_clicked), NULL);
  }

  g_signal_connect(m_combo[COMBOBOX_SORT_ORDER], "changed",
                   G_CALLBACK(combo_changed), gpointer(COMBOBOX_SORT_ORDER));
  g_signal_connect(m_widget, "destroy", G_CALLBACK(destroy_window), NULL);

#if WINDOW_SIZE_TYPE == 0
  gtk_window_maximize(GTK_WINDOW(m_widget));
//	const int height = 680;
//	gtk_widget_set_size_request(m_widget, 16 * height / 10, height);
////notebook resolution 1366x768 40pixels low pane+title
#endif

  m_state = STATE_BEGIN;
  updateStatus();
  gtk_widget_show_all(m_widget);
  gtk_window_set_focus(GTK_WINDOW(m_widget),
                       NULL); // no focus

#if WINDOW_SIZE_TYPE == 1 || WINDOW_SIZE_TYPE == 2
                              //  after gtk_widget_show_all
#if WINDOW_SIZE_TYPE == 1
  const int height = 720; // notebook resolution 1366x768 40pixels low pane,
                          // height - full height of window with title
  const int width = 16 * height / 9; // full hd - 1280x720
#else
                              //  for screenshots on site
  const int width = 780;
  const int height = 10 * width / 16;
#endif

  RECT rect;
  HWND h = GetActiveWindow();
  assert(h != 0 && "GetActiveWindow()!=NULL");
  rect.left = rect.top = rect.right = rect.bottom = 0;
  AdjustWindowRect(&rect, GetWindowLong(h, GWL_STYLE), FALSE);
  gtk_widget_set_size_request(m_widget, width - (rect.right - rect.left),
                              height - (rect.bottom - rect.top));
#endif
}

void Frame::clickMenu(ENUM_MENU menu) {
  GtkTextBuffer *buf;
  GtkTextIter start, end;
  GtkClipboard *clipboard;
  char *text;

  stopThread();

  switch (menu) {

  case MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD:
  case MENU_EDIT_SELECT_ALL:
  case MENU_EDIT_COPY_TO_CLIPBOARD:
    if (menu != MENU_EDIT_COPY_TO_CLIPBOARD) {
      gtk_widget_grab_focus(m_text[TEXTVIEW_MAIN]);
      buf = tvBuffer();
      gtk_text_buffer_get_start_iter(buf, &start);
      gtk_text_buffer_get_end_iter(buf, &end);
      gtk_text_buffer_select_range(buf, &start, &end);
    }

    if (menu != MENU_EDIT_SELECT_ALL) {
      clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
      buf = tvBuffer();
      if (gtk_text_buffer_get_selection_bounds(buf, &start, &end)) {
        text = gtk_text_buffer_get_text(buf, &start, &end,
                                        TRUE); // utf8
        gtk_clipboard_set_text(clipboard, text, -1);
        gtk_clipboard_store(clipboard); // available for other applications
        g_free(text);
      }
    }
    break;

  case MENU_LOAD_ENGLISH_DICTIONARY:
  case MENU_LOAD_RUSSIAN_DICTIONARY:
    clickButton(m_button[BUTTON_DICTIONARY]);
    break;

  case MENU_ABOUT:
    aboutDialog();
    break;

  case MENU_HOMEPAGE:
    openURL(HOMEPAGE);
    break;

  default:
    /* 4.1
     * fixed bug with regular expressions
     * m_menuClick needs only for helper panel
     * other functions should not change m_menuClick
     *
     * earlier user click MENU_REGULAR_EXPRESSIONS, then click MENU_ABOUT and
     * m_menuClick=MENU_ABOUT and if later make some changes for regular
     * expression entry  got exception because m_menuClick=MENU_ABOUT
     *
     * Note if MENU_ENGLISH_LANGUAGE || MENU_RUSSIAN_LANGUAGE
     * need to clear helper so call routine();
     * */
    if (menu == MENU_ENGLISH_LANGUAGE || menu == MENU_RUSSIAN_LANGUAGE) {
      m_languageIndex = menu - MENU_ENGLISH_LANGUAGE;
      loadAndUpdateCurrentLanguage();
    } else {
      m_menuClick = menu;
    }
    /* call only if was last search option, because MENU_ENGLISH_LANGUAGE ||
     * MENU_RUSSIAN_LANGUAGE can be clicked first
     */
    if (m_menuClick != MENU_SEARCH) {
      setHelperPanel();
      routine();
    }
  }
}

void Frame::destroy() {
  writeConfig(CONFIG_TAGS, WORDS_VERSION, m_languageIndex, m_dictionaryIndex,
              m_separatorPosition);
  stopThread();
  gtk_main_quit();
}

void Frame::updateDictionary() {
  int i = getDictionaryIndex(), j = -1;
  for (auto a : {MENU_LOAD_ENGLISH_DICTIONARY, MENU_LOAD_RUSSIAN_DICTIONARY}) {
    j++;
    gtk_widget_set_sensitive(m_menuMap[a], j != i);
  }
  updateButton(BUTTON_DICTIONARY);

  /* additions 4.3
   * need to make new search for every option
   * function updateDictionary() can be called several times before any search
   * option so use m_menuClick as last search option m_menuClick==MENU_SIZE
   * means no last search m_menuClick changes only for search operations see
   * clickMenu() function also stopThreadAndNewRoutine() does entry highlight
   */
  if (m_menuClick != MENU_SEARCH) {
    stopThreadAndNewRoutine();
  }
}

void Frame::aboutDialog() {
  int i;
  size_t j;
  std::string s, s1;
  GtkWidget *box, *hbox, *dialog, *label, *img = gtk_image_new();
  char *markup;
  GdkPixbuf *pi;

  dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog), getMenuLabel(MENU_ABOUT).c_str());
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_container_add(GTK_CONTAINER(hbox), img);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  ENUM_STRING sid[] = {PROGRAM,
                       AUTHOR,
                       COPYRIGHT,
                       HOMEPAGE_STRING,
                       HOMEPAGE_ONLINE_STRING,
                       STRING_SIZE /*build info*/,
                       EXECUTABLE_FILE_SIZE};
  ENUM_STRING id;
  for (i = 0; i < SIZEI(sid); i++) {
    id = sid[i];
    if (id == PROGRAM) {
      s = getProgramVersionString();
    } else if (id == STRING_SIZE) {
      s = getBuildVersionString(false);
    } else {
      s = string(id);

      if (id == AUTHOR) {
        s += " " + string(EMAIL_STRING) + " " + MAIL;
      } else if (id == HOMEPAGE_STRING || id == HOMEPAGE_ONLINE_STRING) {
        s += " " + (id == HOMEPAGE_STRING ? HOMEPAGE : HOMEPAGE_ONLINE);
      } else if (id == COPYRIGHT) {
        j = s.find('(');
        if (j != std::string::npos) {
          s = s.substr(0, j + 1) + "\u00A9" + s.substr(j + 2);
        }
      } else if (id == EXECUTABLE_FILE_SIZE) {
        s += " " + toString(getApplicationFileSize(), ',');
      }
    }

    if (id == HOMEPAGE_STRING || id == HOMEPAGE_ONLINE_STRING) {
      s1 = (id == HOMEPAGE_STRING ? HOMEPAGE : HOMEPAGE_ONLINE) +
           (m_languageIndex ? ',' + getShortLanguageString(m_languageIndex)
                            : "");
      label = gtk_label_new(NULL);
      markup =
          g_markup_printf_escaped("%s <a href=\"%s\">\%s</a>",
                                  string(id).c_str(), s1.c_str(), s1.c_str());
      gtk_label_set_markup(GTK_LABEL(label), markup);
      g_free(markup);
      g_signal_connect(label, "activate-link", G_CALLBACK(label_clicked),
                       gpointer(NULL));

      /*
       //Extra spaces I cann't remove them
       label= gtk_link_button_new (format("%s",HOMEPAGE).c_str());
       g_signal_connect(label, "activate-link",G_CALLBACK(label_clicked), NULL
       );
       //g_object_set (G_OBJECT (label),"image-spacing", 10,NULL);
       addClass(label,"url");
       //css file .url{padding:0px;margin:0px;border:0px;}
       */
    } else {
#ifndef NDEBUG
      // output that NDEBUG is not defined
      if (i == 0) {
        s += " DEBUG VERSION";
      }
#endif
      label = createLabel(s);
    }

    gtk_widget_set_halign(label, GTK_ALIGN_START);
    addClass(label, "aboutlabel");

    add(box, label); // stretch vertically
    // gtk_container_add(GTK_CONTAINER(box), label);
  }

  gtk_container_add(GTK_CONTAINER(hbox), box);
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);

  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_widget));

  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_widget_show_all(dialog);

  // get height after show_all
  gtk_widget_get_preferred_height(box, nullptr, &i);
  pi = gdk_pixbuf_new_from_file_at_size(getImagePath("word256.png").c_str(), i,
                                        i, 0);
  gtk_image_set_from_pixbuf(GTK_IMAGE(img), pi);
  g_object_unref(pi);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

void Frame::routine(bool full) {
  bool b = prepare();
  m_state = b ? STATE_PROCEEDING : STATE_ERROR;
  if (full) {
    SearchResult::out = "";
    m_result.clear();
  }
  clearTagMarks();
  m_begin = clock();
  m_addstatus = "";
  m_filteredWordsCount = 0; // need to set always because in case of error
                            // need m_filteredWordsCount = 0
  setLabel(m_searchTagLabel, "");
  if (!b) {
    m_end = clock();
  }
  updateStatus(); // before thread

  if (b) {
    startThread(full);
  }
}

void Frame::setHelperPanel() {
  GtkWidget *w, *w1;
  gchar *p;
  std::string s;

  clearContainer(m_helperUp);

  // set label
  auto it = MENU_TO_HELP_STRING.find(m_menuClick);
  if (it !=
      MENU_TO_HELP_STRING
          .end()) { // for some of menu items only needs clear helper panel
    w = gtk_label_new("");
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
    gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_FILL);
    gtk_label_set_line_wrap(GTK_LABEL(w), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(w), 40);

    s = replaceAll(string(it->second), "<br>", "\n");
    p = g_markup_printf_escaped(s.c_str());
    gtk_label_set_markup(GTK_LABEL(w), p);
    g_free(p);
  }

  ENUM_STRING e = entryEnumString();
  if (e != STRING_SIZE) {
    w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_container_add(GTK_CONTAINER(w), createLabel(SEARCH));
    m_entry[ENTRY_TEMPLATE] = w1 = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(w1), stringUsingDictionary(e).c_str());
    connectEntrySignals(ENTRY_TEMPLATE);
    add(w, w1);
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
  }

  switch (m_menuClick) {
  case MENU_ANAGRAM:
    addComboLineToHelper(LENGTH, 2, MAX_ANAGRAM_LENGTH, 6, CHARACTERS);
    break;

  case MENU_PANGRAM:
    w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN);
    add(w, string(MINIMUM) + " " + string(DIFFERENT_CHARACTERS));
    add(w, createTextCombo(COMBOBOX_HELPER0, 10, MAX_PANGRAM_LENGTH, 5));
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
    break;

  case MENU_TEMPLATE:
    // i've changed first selection (since version 4.0) from 2 to 0 for
    // description & combobox agreement
    addComboToHelper(TEMPLATE1, TEMPLATE3, 0);
    break;

  case MENU_REGULAR_EXPRESSIONS:
    addComboLineToHelper(NUMBER_OF_MATCHES, 1, 10, 0, STRING_SIZE, true);
    break;

  case MENU_MODIFICATION:
    m_check = gtk_check_button_new_with_label(
        string(EVERY_MODIFICATION_CHANGES_WORD).c_str());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_check), TRUE);
    gtk_container_add(GTK_CONTAINER(m_helperUp), m_check);
    g_signal_connect(m_check, "toggled", G_CALLBACK(check_changed), NULL);
    break;

  case MENU_CHAIN:
    w = createLabel(EXCEPTION_WORDS);
    gtk_widget_set_halign(w, GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);

    w = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    m_text[TEXTVIEW_HELPER] = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_text[TEXTVIEW_HELPER]),
                                GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(w), m_text[TEXTVIEW_HELPER]);
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
    updateTextView(TEXTVIEW_HELPER,
                   stringUsingDictionary(SETTINGS_CHAIN_EXCEPTIONS));
    g_signal_connect(tvBuffer(TEXTVIEW_HELPER), "changed",
                     G_CALLBACK(text_view_changed), NULL);

    break;

  case MENU_CHARACTER_SEQUENCE:
    addComboToHelper(SEARCH_IN_ANY_PLACE_OF_WORD, SEARCH_IN_END_OF_WORD, 0,
                     COMBOBOX_HELPER2);
    addComboLineToHelper(NUMBER_OF_MATCHES, 1, 10, 0, STRING_SIZE, true);
    break;

  case MENU_SIMPLE_WORD_SEQUENCE:
    addComboLineToHelper(SEQUENCE, 8, MAX_SIMPLE_WORD_SEQUENCE_LENGTH, 0,
                         CHARACTERS);
    break;

  case MENU_DOUBLE_WORD_SEQUENCE:
    addComboLineToHelper(LENGTH, 2, MAX_DOUBLE_WORD_SEQUENCE_LENGTH, 2,
                         CHARACTERS);
    break;

  case MENU_CONSONANT_VOWEL_SEQUENCE:
    w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN);
    assert(VOWELS + 1 == CONSONANTS);
    add(w, createTextCombo(COMBOBOX_HELPER0, SEARCH_IN_ANY_PLACE_OF_WORD,
                           SEARCH_IN_END_OF_WORD, 0));
    add(w, createTextCombo(COMBOBOX_HELPER1, 3, 10, 0));
    add(w, createTextCombo(COMBOBOX_HELPER2, VOWELS, CONSONANTS, 0));
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
    break;

  case MENU_DENSITY:
    w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN);
    add(w, MAXIMUM);
    add(w, createTextCombo(COMBOBOX_HELPER0, 0, 25, 25));
    add(w, "%");
    add(w, createTextCombo(COMBOBOX_HELPER1, VOWELS, CONSONANTS, 0));
    add(w, CHARACTERS);
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
    break;

    // otherwise nothing to do
  default:;
  }

  gtk_widget_show_all(m_helperUp);
}

void Frame::loadAndUpdateCurrentLanguage() {
  std::string s;
  int i = -1;
  for (auto &a : m_menuAll[m_languageIndex]) {
    i++;
    setMenuLabel(ENUM_MENU(i), a);
  }

  i = -1;
  for (auto &a : {MENU_ENGLISH_LANGUAGE, MENU_RUSSIAN_LANGUAGE}) {
    i++;
    gtk_widget_set_sensitive(m_menuMap[a], m_languageIndex != i);
  }

  setPlaceholder(ENTRY_SEARCH, SEARCH);
  setLabel(m_currentDictionary, DICTIONARY);
  gtk_window_set_title(GTK_WINDOW(m_widget), string(PROGRAM).c_str());
  setPlaceholder(ENTRY_FILTER, RESULTS_FILTER);
  refillCombo(COMBOBOX_SORT, SORT_BY_ALPHABET, NUMBER_OF_SORTS);
  refillCombo(COMBOBOX_FILTER, FOUND, 2);
}

GtkWidget *Frame::createTextCombo(ENUM_COMBOBOX e, VString v, int active) {
  assert(e != COMBOBOX_SIZE);

  GtkWidget *w = m_combo[e] = gtk_combo_box_text_new();
  for (auto &a : v) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(w), a.c_str());
  }

  setComboIndex(e, active);
  g_signal_connect(w, "changed", G_CALLBACK(combo_changed), gpointer(e));
  return w;
}

GtkWidget *Frame::createTextCombo(ENUM_COMBOBOX e) {
  return createTextCombo(e, {}, -1);
}

GtkWidget *Frame::createTextCombo(ENUM_COMBOBOX e, int from, int to,
                                  int active) {
  VString v;
  int i;
  for (i = from; i <= to; i++) {
    v.push_back(std::to_string(i));
  }
  return createTextCombo(e, v, active);
}

GtkWidget *Frame::createTextCombo(ENUM_COMBOBOX e, ENUM_STRING from,
                                  ENUM_STRING to, int active) {
  VString v;
  int i;
  for (i = from; i <= to; i++) {
    v.push_back(string(i));
  }
  return createTextCombo(e, v, active);
}

void Frame::addComboLineToHelper(ENUM_STRING id, int from, int to, int active,
                                 ENUM_STRING eid, bool any) {
  addComboLineToHelper(id, from, to, active, string(id), string(TO),
                       eid == STRING_SIZE ? "" : string(eid), any);
}

void Frame::addComboLineToHelper(ENUM_STRING id, int from, int to, int active,
                                 std::string s1, std::string s2, std::string s3,
                                 bool any) {
  int i, j;
  GtkWidget *w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN), *r;
  for (i = 0; i < 2; i++) {
    add(w, i == 0 ? s1 + (any ? "" : " " + string(FROM)) : s2);
    if (!i && any) {
      m_radioValue = 0;
      for (j = 0; j < 2; j++) {
        if (j == 0) {
          m_radio = r =
              gtk_radio_button_new_with_label(NULL, string(ANY).c_str());
        } else {
          r = gtk_radio_button_new_with_label_from_widget(
              GTK_RADIO_BUTTON(m_radio), string(FROM).c_str());
        }
        add(w, r);
        g_signal_connect(r, "toggled", G_CALLBACK(radio_changed), NULL);
      }
    }
    add(w, createTextCombo(HELPER_COMBOBOX[i], from, to, active));
  }
  if (!s3.empty()) {
    add(w, s3);
  }
  gtk_container_add(GTK_CONTAINER(m_helperUp), w);
  m_comboline = w;
}

void Frame::addComboToHelper(ENUM_STRING from, ENUM_STRING to, int active,
                             ENUM_COMBOBOX comboboxId /*=COMBOBOX_HELPER0*/) {
  gtk_container_add(GTK_CONTAINER(m_helperUp),
                    createTextCombo(comboboxId, from, to, active));
}

void Frame::comboChanged(ENUM_COMBOBOX e) {
  assert(e != COMBOBOX_SIZE);
  updateComboValue(e);

  if (m_menuClick == MENU_CHARACTER_SEQUENCE && e == COMBOBOX_HELPER2) {
    // visible is "any place of word"
    gtk_widget_set_visible(m_comboline, getComboIndex(e) == 0);
  }

  if (oneOf(e, COMBOBOX_SORT, COMBOBOX_SORT_ORDER, COMBOBOX_FILTER)) {
    stopThreadAndNewRoutine(false);
    return;
  }

  stopThread();
  if ((e == COMBOBOX_HELPER0 || e == COMBOBOX_HELPER1) &&
      oneOf(m_menuClick, MENU_ADJUST_COMBO)) {
    if (getComboIndex(COMBOBOX_HELPER0) > getComboIndex(COMBOBOX_HELPER1)) {
      lockSignals();
      setComboIndex(e == COMBOBOX_HELPER0 ? COMBOBOX_HELPER1 : COMBOBOX_HELPER0,
                    getComboIndex(e));
      unlockSignals();
    }
  }
  // for COMBOBOX_HELPER0-2
  routine();
}

void Frame::createImageCombo(ENUM_COMBOBOX e) {
  int i;
  GtkTreeIter iter;
  GtkCellRenderer *renderer;
  const char *image[] = {"ascending.png", "descending.png"};
  GtkListStore *gls = gtk_list_store_new(1, GDK_TYPE_PIXBUF);
  for (i = 0; i < 2; i++) {
    gtk_list_store_append(gls, &iter);
    gtk_list_store_set(gls, &iter, 0, pixbuf(image[i]), -1);
  }
  m_combo[e] = gtk_combo_box_new_with_model(GTK_TREE_MODEL(gls));
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_combo[e]), renderer, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(m_combo[e]), renderer,
                                 "pixbuf", 0, NULL);
}

void Frame::clickButton(GtkWidget *button) {
  int i, n = indexOf(button, m_button);
  if (n == BUTTON_DICTIONARY) {
    m_dictionaryIndex = !m_dictionaryIndex;
    updateDictionary();
  } else if (n == BUTTON_STARTSTOP) {
    auto b = getStartStopState();
    if (b.imageStart) {
      prsync("start");
      routine();
    } else {
      m_thread.request_stop();
      m_state = STATE_STOPPING;
      updateStatus();
    }
  } else {
    if (m_tags < 2) {
      return;
    }
    i = m_tagIndex + (n == BUTTON_NEXT ? 1 : m_tags - 1);
    updateTags(i % m_tags);
  }
}

void Frame::setMenuLabel(ENUM_MENU e, std::string const &text) {
  GtkWidget *w = m_menuMap[e];
  if (MENU_TO_ICON_FILE.contains(e)) {
    w = gtk_bin_get_child(GTK_BIN(w));
    GList *list = gtk_container_get_children(GTK_CONTAINER(w));
    assert(g_list_length(list) == 2);

    gtk_label_set_label(GTK_LABEL(g_list_nth(list, 1)->data), text.c_str());
  } else {
    gtk_menu_item_set_label(GTK_MENU_ITEM(w), text.c_str());
  }
}

void Frame::endJobThread() { gdk_threads_add_idle(end_job, NULL); }

std::string Frame::getMenuLabel(ENUM_MENU e) {
  GtkWidget *w = m_menuMap[e];
  if (MENU_TO_ICON_FILE.contains(e)) {
    w = gtk_bin_get_child(GTK_BIN(w));
    GList *list = gtk_container_get_children(GTK_CONTAINER(w));
    assert(g_list_length(list) == 2);
    return gtk_label_get_label(GTK_LABEL(g_list_nth(list, 1)->data));
  } else {
    return gtk_menu_item_get_label(GTK_MENU_ITEM(w));
  }
}

void Frame::endJob() {
  // make unjoinable
  if (m_thread.joinable())
    m_thread.join();
  updateStatus();
  // update tags if user searched something
  updateTags(0);
  if (m_currentEntry != ENTRY_SIZE) { // means calls from entry changed,restor focus and position
    gtk_widget_grab_focus(m_entry[m_currentEntry]);
    gtk_editable_set_position(GTK_EDITABLE(m_entry[m_currentEntry]),
                              m_currentEntryPos);
    m_currentEntry = ENTRY_SIZE; // reset value
  }
}

void Frame::stopThreadAndNewRoutine(bool full) {
  if (!full && m_result.empty()) { // only sort
    return;
  }
  stopThread();
  routine(full);
}

/**
 * if thread runs stop it
 */
void Frame::stopThread() {
  m_thread.request_stop();
  if (m_thread.joinable()) {
    m_thread.join();
  }
}

void Frame::startThread(bool full) {
  if (!m_thread.joinable()) {
    // GCC bug #100612 so use lambda if call class member
    m_thread = std::jthread([this, full](std::stop_token token) {
      m_token = token;
      run(full);
    });
  } else {
    pr("error start thread joinable")
  }
}

gint Frame::getComboIndex(ENUM_COMBOBOX e) const {
  assert(e != COMBOBOX_SIZE);
  assert(GTK_IS_COMBO_BOX(m_combo[e]));
  return gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[e]));
}

void Frame::setComboIndex(ENUM_COMBOBOX e, gint v) {
  assert(GTK_IS_COMBO_BOX(m_combo[e]));
  gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[e]), v);
  updateComboValue(e);
}

void Frame::updateComboValue(ENUM_COMBOBOX e) {
  assert(e != COMBOBOX_SIZE);
  int v = -1;
  if (GTK_IS_COMBO_BOX_TEXT(m_combo[e])) {
    char *p =
        gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_combo[e]));
    if (p && parseString(p, v)) {
      assert(v >= 0);
    }
  }
  if (v == -1) {
    v = getComboIndex(e);
    // v can be =-1 when combobox just created
  }
  m_comboValue[e] = v;
}

void Frame::connectEntrySignals(ENUM_ENTRY e) {
  GCallback f[] = {G_CALLBACK(entry_insert), G_CALLBACK(entry_delete),
                   G_CALLBACK(entry_focus_in), G_CALLBACK(entry_focus_out)};
  int i = 0;
  for (auto a :
       {"insert-text", "delete-text", "focus-in-event", "focus-out-event"}) {
    g_signal_connect_after(G_OBJECT(m_entry[e]), a, f[i++], GINT_TO_POINTER(e));
  }
}

void Frame::entryFocusChanged(bool in) {
  if (in) {
    removeAccelerators();
  } else {
    addAccelerators();
  }
}

void Frame::removeAccelerators() {
  for (auto &a : m_accelGroup) {
    gtk_window_remove_accel_group(GTK_WINDOW(m_widget), a);
  }
}

/**
 * Note even if program use accelerators and combobox is active then hotkeys
 * don't work
 */
void Frame::addAccelerators() {
  for (auto &a : m_accelGroup) {
    gtk_window_add_accel_group(GTK_WINDOW(m_widget), a);
  }
}

void Frame::refillCombo(ENUM_COMBOBOX e, ENUM_STRING first, int length) {
  int i, j = getComboIndex(e);
  if (j == -1) { // was empty combo
    if (e == COMBOBOX_SORT) {
      // sort by length descendant
      j = 1;
    } else {
      // regex filter "match" option - default filter
      j = 0;
    }
  }
  lockSignals();
  auto c = GTK_COMBO_BOX_TEXT(m_combo[e]);
  gtk_combo_box_text_remove_all(c);
  for (i = 0; i < length; i++) {
    gtk_combo_box_text_append_text(c, string(first + i).c_str());
  }
  setComboIndex(e, j);
  unlockSignals();
}

void Frame::newVersionMessage() {
  std::string s = string(NEW_VERSION_MESSAGE) + "\n" + m_newVersion.m_message;
  GtkWidget *d =
      gtk_message_dialog_new(GTK_WINDOW(m_widget), GTK_DIALOG_MODAL,
                             GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, s.c_str());

  gtk_window_set_title(GTK_WINDOW(d), getProgramVersionString().c_str());
  gint result = gtk_dialog_run(GTK_DIALOG(d));
  gtk_widget_destroy(d);

  if (result == GTK_RESPONSE_YES) {
    openURL(DOWNLOAD_URL);
  }
}

void Frame::radioChanged(GtkWidget *w) {
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
    GSList *group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(w));
    m_radioValue = g_slist_length(group) - 1 -
                   g_slist_index(group, w); // Note group inverted order
    stopThreadAndNewRoutine();
  }
}

void Frame::updateTextView(ENUM_TEXTVIEW e, std::string const &s) {
  gtk_text_buffer_set_text(tvBuffer(e), s.c_str(), -1);
}

void Frame::setDebounceTimer(ENUM_ENTRY e) {
  if (m_debounceTimerId) {
    g_source_remove(m_debounceTimerId);
  }
  m_debounceTimerId =
      g_timeout_add(TIMER, on_debounce_timeout, GINT_TO_POINTER(e));
}

void Frame::debounceTimeout(ENUM_ENTRY e) {
  m_debounceTimerId = 0;
  m_currentEntry =
      e; // store current entry, because after thread loose focus and cursor
  m_currentEntryPos = gtk_editable_get_position(GTK_EDITABLE(m_entry[e]));
  prs(m_currentEntryPos);

  switch (e) {
  case ENTRY_TEMPLATE:
    stopThreadAndNewRoutine();
    break;

  case ENTRY_SEARCH:
    updateTags(0);
    break;

  case ENTRY_FILTER:
    if (m_regex[ENTRY_FILTER]) {
      stopThreadAndNewRoutine(false);
    }
    break;

  default:
    assert(0);
  }
}

void Frame::setLabel(GtkWidget *w, ENUM_STRING e) { setLabel(w, string(e)); }

void Frame::setLabel(GtkWidget *w, const std::string &s) {
  gtk_label_set_text(GTK_LABEL(w), s.c_str());
}

// lowercased utf8, changed 'ё' -> 'е'
std::string Frame::getEntryString(ENUM_ENTRY e) const {
  const gchar *p = gtk_entry_get_text(GTK_ENTRY(m_entry[e]));
  gchar *lower_str = g_utf8_strdown(p, -1);
  // 'ё' -> 'е'
  gchar *cursor = lower_str;
  while (*cursor != '\0') {
    if ((guchar)cursor[0] == 0xD1 && (guchar)cursor[1] == 0x91) {
      cursor[0] = 0xD0;
      cursor[1] = 0xB5;
    }
    cursor = g_utf8_next_char(cursor);
  }
  std::string s = lower_str;
  g_free(lower_str);
  return s;
}

std::string Frame::getTextViewString() const {
  GtkTextBuffer *buffer = tvBuffer(TEXTVIEW_HELPER);
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gchar *raw_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  auto s = utf8ToLocale(raw_text);
  g_free(raw_text);
  return s;
}

bool Frame::getCheck() const {
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_check)) == TRUE;
}

void Frame::entryChanged(ENUM_ENTRY e) {
  bool b;
  if (e == ENTRY_TEMPLATE) {
    b = prepare();
    addRemoveClass(m_entry[e], CERROR, !b);
  } else if (e == ENTRY_FILTER) {
    clearTagMarks();
    b = createRegex(e);
    addRemoveClass(m_entry[e], CERROR, !b);
  } else if (e == ENTRY_SEARCH) {
    clearTagMarks();
  }

  setDebounceTimer(e);
}

void Frame::clearTagMarks() {
  GtkTextBuffer *buffer = tvBuffer();
  for (auto &range : m_found_tags) {
    gtk_text_buffer_delete_mark(buffer, range.start_mark);
    gtk_text_buffer_delete_mark(buffer, range.end_mark);
  }
  m_found_tags.clear();
}

// n - number of active tag
void Frame::updateTags(int n) {
  GtkTextBuffer *buffer = tvBuffer();
  std::string s = getEntryString(ENTRY_SEARCH);

  int old_tag_index = m_tagIndex;
  m_tagIndex = n;

  if (!s.empty() && !m_found_tags.empty() && m_tags > 0) {

    if (old_tag_index >= 0 && old_tag_index < (int)m_found_tags.size()) {
      GtkTextIter s_iter, e_iter;
      gtk_text_buffer_get_iter_at_mark(buffer, &s_iter,
                                       m_found_tags[old_tag_index].start_mark);
      gtk_text_buffer_get_iter_at_mark(buffer, &e_iter,
                                       m_found_tags[old_tag_index].end_mark);

      gtk_text_buffer_remove_tag_by_name(buffer, activeTag, &s_iter, &e_iter);
      gtk_text_buffer_apply_tag_by_name(buffer, markTag, &s_iter, &e_iter);
    }

    GtkTextIter scroll;
    if (m_tagIndex >= 0 && m_tagIndex < (int)m_found_tags.size()) {
      GtkTextIter s_iter, e_iter;
      gtk_text_buffer_get_iter_at_mark(buffer, &s_iter,
                                       m_found_tags[m_tagIndex].start_mark);
      gtk_text_buffer_get_iter_at_mark(buffer, &e_iter,
                                       m_found_tags[m_tagIndex].end_mark);

      gtk_text_buffer_remove_tag_by_name(buffer, markTag, &s_iter, &e_iter);
      gtk_text_buffer_apply_tag_by_name(buffer, activeTag, &s_iter, &e_iter);
      scroll = s_iter;
    }

    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(m_text[TEXTVIEW_MAIN]), &scroll,
                                 0.0, true, .5, .5);

    setLabel(m_searchTagLabel,
             std::format("{}/{}", m_tags == 0 ? 0 : m_tagIndex + 1, m_tags));
    return;
  }

  GtkTextIter first, scroll, start, end;
  gint i, j;

  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gtk_text_buffer_remove_all_tags(buffer, &start, &end);
  clearTagMarks();

  if (s.empty()) {
    setLabel(m_searchTagLabel, "");
    return;
  }

  VString v = split(gtk_text_iter_get_text(&start, &end), "\n");
  const gchar *text = s.c_str();

  gtk_text_buffer_get_start_iter(buffer, &first);
  for (m_tags = 0; gtk_text_iter_forward_search(
           &first, text,
           GtkTextSearchFlags(GTK_TEXT_SEARCH_TEXT_ONLY |
                              GTK_TEXT_SEARCH_CASE_INSENSITIVE |
                              GTK_TEXT_SEARCH_VISIBLE_ONLY),
           &start, &end, NULL);
       gtk_text_buffer_get_iter_at_offset(buffer, &first,
                                          gtk_text_iter_get_offset(&end))) {

    i = gtk_text_iter_get_line(&start);
    j = gtk_text_iter_get_line_offset(&start);
    auto p = v[i].find(OPEN_BRACKET);
    if (p != std::string::npos) {
      i = g_utf8_strlen(v[i].c_str(), p);
      if (j >= i) {
        continue;
      }
    }

    GtkTextMark *sm = gtk_text_buffer_create_mark(buffer, NULL, &start, TRUE);
    GtkTextMark *em = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
    m_found_tags.push_back({sm, em});

    if (m_tags == m_tagIndex) {
      scroll = start;
    }

    gtk_text_buffer_apply_tag_by_name(
        buffer, m_tags == m_tagIndex ? activeTag : markTag, &start, &end);
    m_tags++;
  }

  if (m_tags > 0) {
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(m_text[TEXTVIEW_MAIN]), &scroll,
                                 0.0, true, .5, .5);
  }
  setLabel(m_searchTagLabel,
           std::format("{}/{}", m_tags == 0 ? 0 : m_tagIndex + 1, m_tags));
}

GtkTextBuffer *Frame::tvBuffer(ENUM_TEXTVIEW e) const {
  return gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text[e]));
}

std::string Frame::getProgramVersionString() const {
  return string(PROGRAM) + " " + string(VERSION) + " " + WORDS_VERSION;
}

void Frame::updateStatus() {
  // pr(magic_enum::enum_name(m_state));
  bool b = true;
  switch (m_state) {
  case STATE_BEGIN:
    m_out = "";
    break;

  case STATE_OK:
    if (SearchResult::out.size()) {
      // SearchResult::out can be too big so can't copy
      // m_out = std::move(SearchResult::out); also not possible need to store
      // SearchResult::out
      b = false;
    } else {
      m_out = string(NO_RESULTS_FOUND);
    }
    break;

  case STATE_PROCEEDING:
    m_out = string(oneOf(m_menuClick, MENU_WAITING) ? WAITING : SEARCH);
    break;

  case STATE_ERROR:
    m_out = string(STRING_ERROR);
    break;

  case STATE_USER_BREAK:
    m_out = string(OPERATION_CANCELED_BY_USER);
    break;

  case STATE_STOPPING:
    m_out = string(STOPPING_PROCESS);
    break;
  }

  std::string s = m_state == STATE_OK ? getStatusString() : m_out;
  setLabel(m_statusMessage, s);

  if (b && m_state != STATE_BEGIN) {
    m_out = capitalizeFirstUtf8(m_out) +
            (oneOf(m_state, STATE_PROCEEDING, STATE_STOPPING) ? "…" : ".");
  }
  updateTextView(TEXTVIEW_MAIN, b ? m_out : SearchResult::out);

  b = m_state == STATE_OK && !m_result.empty();
  setSensitiveOrderFilter(b);
  updateButton(BUTTON_STARTSTOP);
}

void Frame::setSensitiveOrderFilter(bool b) {
  for (auto &e : {COMBOBOX_SORT, COMBOBOX_SORT_ORDER, COMBOBOX_FILTER}) {
    gtk_widget_set_sensitive(m_combo[e], b);
  }
  gtk_widget_set_sensitive(m_entry[ENTRY_FILTER], b);
}

void Frame::setPlaceholder(ENUM_ENTRY e, ENUM_STRING s) {
  gtk_entry_set_placeholder_text(GTK_ENTRY(m_entry[e]), string(s).c_str());
}

void Frame::updateButton(ENUM_BUTTON e) {
  std::string s;
  if (e == BUTTON_STARTSTOP) {
    auto b = getStartStopState();
    gtk_widget_set_sensitive(m_button[e], b.enable);
    s = b.imageStart ? "play.png" : "stop.png";

  } else if (e == BUTTON_DICTIONARY) {
    s = m_dictionaryIndex ? "ru.gif" : "en.gif";
  }
  gtk_button_set_image(GTK_BUTTON(m_button[e]), image(s));
}

StartStopButtonState Frame::getStartStopState() const {
  return {m_state != STATE_PROCEEDING,
          !oneOf(m_state, STATE_ERROR, STATE_BEGIN)};
}

GtkWidget *Frame::createBox(GtkOrientation o, int margin, WB wb) {
  auto w = gtk_box_new(o, margin);
  for (auto &e : wb) {
    add(w, e.first, e.second);
  }
  return w;
}
