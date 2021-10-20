/*
 * Frame.cpp
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#include "Frame.h"
#include "CheckNewVersion.h"

/*
 * WINDOW_SIZE_TYPE=0 default
 * WINDOW_SIZE_TYPE=1 for youtube helper clip writing
 * WINDOW_SIZE_TYPE=2 for site screenshots
 */
#define WINDOW_SIZE_TYPE 0

//Note WORDS_VERSION defined in WordsBase
const char MAIL[] = "slovesnov@yandex.ru";
const char HOMEPAGE[] = "http://slovesnov.users.sf.net/?words"; //sf shorter than sourceforge, better looks in about dialog
const char HOMEPAGE_ONLINE[] = "http://slovesnov.users.sf.net/?words_online"; //sf shorter than sourceforge, better looks in about dialog
const char markTag[] = "mark";
const char activeTag[] = "active";
const int SEARCH_ENTRY_ID = -1;
const int FILTER_ENTRY_ID =-2;
const char *MONTH[] = { "January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December" };
const char CERROR[] = "cerror";

const char DOWNLOAD_URL[] =
		"http://sourceforge.net/projects/javawords/files/latest/download";
//const char DOWNLOAD_URL[]="https://sourceforge.net/projects/javawords/files/";//all versions


const char VERSION_FILE_URL[] =
		"http://slovesnov.users.sf.net/words/version.txt";

//const char VERSION_FILE_URL[] = "file:///C:/slovesno/site/words/version.txt";
/* too many items in combobox, so set maximum bound.
 * values counted from dictionary
 * makes the same for all dictionaries to avoid refill after
 * dictionary changes
 */
const int MAX_ANAGRAM_LENGTH = 31; //counted {18,31}
const int MAX_PANGRAM_LENGTH = 20; //counted {16,20}
const int MAX_WORD_SEQUENCE_LENGTH = 21; //counted {20,21}
const int MAX_DOUBLE_WORD_SEQUENCE_LENGTH = 14; //counted {6,14}

const std::string CONFIG_TAGS[]={"version","language","dictionary"};

Frame *frame;

static gpointer thread(gpointer) {
	frame->proceedThread();
	return NULL;
}

static gpointer sort_filter_thread(gpointer) {
	frame->sortFilterAndUpdateResults();
	return NULL;
}

static gboolean end_job(gpointer) {
	frame->endJob();
	return G_SOURCE_REMOVE;
}

static void menu_activate(GtkWidget *widget, ENUM_MENU menu) {
	frame->clickMenu(menu);
}

static void combo_changed(GtkComboBox *comboBox, ENUM_COMBOBOX e) {
	if (!frame->isSignalsLocked()) {
		frame->comboChanged(e);
	}
}

static void entry_insert(GtkWidget *entry, gchar *new_text,
		gint new_text_length, gpointer position, int entryIndex) {
	frame->entryChanged(entryIndex);
}

static void entry_delete(GtkWidget *entry, gint start_pos, gint end_pos,
		int entryIndex) {
	frame->entryChanged(entryIndex);
}

static gboolean entry_focus_in(GtkWidget *widget, GdkEvent *event, gpointer) {
	frame->entryFocusChanged(true);
	return TRUE;
}

static gboolean entry_focus_out(GtkWidget *widget, GdkEvent *event, gpointer) {
	frame->entryFocusChanged(false);
	return TRUE;
}

static void button_clicked(GtkWidget *button, gpointer) {
	frame->clickButton(button);
}

static gboolean label_clicked(GtkWidget *label, const gchar *uri, gpointer) {
	openURL(uri);
	return TRUE;
}

static void check_changed(GtkWidget *check, gpointer) {
	frame->checkChanged(check);
}

static void destroy_window(GtkWidget *object, gpointer) {
	frame->destroy();
}

static gboolean new_version_message(gpointer) {
	frame->newVersionMessage();
	return G_SOURCE_REMOVE;
}

Frame::Frame() :
		WordsBase() {
	GtkWidget *w, *w1, *w2, *scroll;
	GtkWidget *item;
	FILE *f;
	char buff[MAX_BUFF_LEN];
	bool bSubMenu;
	int i, j;
	std::vector<GtkMenuItem*> subMenu;
	std::string s;

	static_assert(SIZE(ICON_MENU)==SIZE(ICON_MENU_FILE_NAME));
	static_assert(SIZE(HELPER_MENU)==SIZE(HELPER_STRING));
	static_assert(SIZE(FUNCTION_MENU)==SIZE(FUNCTION_ID));
	static_assert(SIZE(BOOL_VOID_MENU)==SIZE(BOOL_VOID_FUNCTION));
	static_assert(MENU_ACCEL_SIZE==SIZE(ACCEL_KEY));

#ifndef NDEBUG
	//No intersection between FUNCTION_MENU & BOOL_VOID_MENU
	for(i=0;i<SIZEI(FUNCTION_MENU);i++){
		if(ONE_OF(BOOL_VOID_MENU,FUNCTION_MENU[i])){
			assert(0);
		}
	}
#endif

	frame = this;
	m_menuClick = MENU_SEARCH;
	//set dot as decimal separator, standard locale
	setlocale(LC_NUMERIC, "C");
	m_lockSignals = false;
	m_thread = 0;
	g_mutex_init(&m_mutex);

	m_widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	m_menu = gtk_menu_bar_new();
	m_text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_text), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_text), FALSE);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text)),
			markTag, "background", "lightblue", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text)),
			activeTag, "background", "Khaki", NULL);
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scroll), m_text);

	m_helperUp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	m_status = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	m_statusMessage = gtk_label_new("");

	/*
	 #ifdef NDEBUG
	 s="no debug";
	 #else
	 s="debug";
	 #endif
	 gtk_label_set_text(GTK_LABEL(m_statusMessage),s.c_str());
	 */

	createImageCombo(COMBOBOX_SORT_ORDER);
	createImageCombo(COMBOBOX_DICTIONARY);

	createTextCombo(COMBOBOX_SORT);
	setComboIndex(COMBOBOX_SORT_ORDER, 1);

	createTextCombo(COMBOBOX_FILTER);

	m_searchLabel = gtk_label_new("");
	m_searchEntry = gtk_entry_new();
	m_searchTagLabel = gtk_label_new("");
	for (i = 0; i < SIZEI(m_searchButton); i++) {
		m_searchButton[i] = gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(m_searchButton[i]),image(i == 0 ? "down.png" : "up.png"));
	}
	m_currentDictionary = gtk_label_new("");
	m_filterEntry = gtk_entry_new();

	for (i = 0; i < MENU_ACCEL_SIZE; i++) {
		m_accelGroup[i] = gtk_accel_group_new();
	}
	addAccelerators();

	//fill containers
	const int margin = 4;
	add(m_status, m_statusMessage);
	gtk_widget_set_halign(m_statusMessage, GTK_ALIGN_START);

	//sort combo has many items so place it into the middle
	w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
	add(w1, m_combo[COMBOBOX_SORT]);
	gtk_container_add(GTK_CONTAINER(w1), m_combo[COMBOBOX_SORT_ORDER]);
	gtk_container_add(GTK_CONTAINER(w), w1);

	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
	gtk_container_add(GTK_CONTAINER(w1), m_searchLabel);
	add(w1, m_searchEntry); //stretch
	gtk_container_add(GTK_CONTAINER(w1), m_searchTagLabel);
	for (i = 0; i < SIZEI(m_searchButton); i++) {
		gtk_container_add(GTK_CONTAINER(w1), m_searchButton[i]);
	}
	gtk_container_add(GTK_CONTAINER(w1), m_currentDictionary);
	gtk_container_add(GTK_CONTAINER(w1), m_combo[COMBOBOX_DICTIONARY]);
	gtk_container_add(GTK_CONTAINER(w), w1);

	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
	//gtk_container_add(GTK_CONTAINER(w1), m_filterLabel);
	add(w1, m_filterEntry); //stretch
	gtk_container_add(GTK_CONTAINER(w1), m_combo[COMBOBOX_FILTER]);

	m_filterFrame= gtk_frame_new("");
	gtk_container_add(GTK_CONTAINER(m_filterFrame), w1);
	gtk_frame_set_label_align(GTK_FRAME(m_filterFrame), 0.15, 0.5);
	gtk_container_add(GTK_CONTAINER(w), m_filterFrame);

	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
	w2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	add(w2, scroll);
	gtk_container_add(GTK_CONTAINER(w2), m_status);

	add(w1, w2);

	w2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_container_add(GTK_CONTAINER(w2), m_helperUp);
	add(w2, ""); //use as glue on java
	gtk_container_add(GTK_CONTAINER(w2), w);

	gtk_container_add(GTK_CONTAINER(w1), w2);
	gtk_widget_set_margin_end(m_helperUp, margin);

	w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(w), m_menu);
	add(w, w1);
	gtk_container_add(GTK_CONTAINER(m_widget), w);

	//load configuration file
	int dictionary=0;
	m_languageIndex = 0;

	MapStringString m;
	MapStringString::iterator it;
	if (loadConfig(m)) {
		for (j = 0; j < 2; j++) {
			if ((it = m.find(j == 0 ? "language" : "dictionary")) != m.end()) {
				for (i = 0; i < LANGUAGES; i++) {
					if (getShortLanguageString(i) == it->second) {
						if (j == 0) {
							m_languageIndex = i;
						} else {
							dictionary = i;
						}
						break;
					}
				}
			}
		}
	}

	//load menu
	f = open(m_languageIndex, "language");
	assert(f!=NULL);
	i = 0;
	while (fgets(buff, MAX_BUFF_LEN, f) != NULL) {
		if (subMenu.empty() && strlen(buff) == 1) { //strlen(buff)==1 ~ buff="\n"
			break;
		}

		if (startsWith(buff, SEPARATOR)) {
			gtk_menu_shell_append(
					GTK_MENU_SHELL(gtk_menu_item_get_submenu(subMenu.back())),
					gtk_separator_menu_item_new());
			continue;
		}

		if (strchr(buff, '}') != NULL) {
			subMenu.pop_back();
			continue;
		}

		bSubMenu = strchr(buff, '{') != NULL;

		if ((j = INDEX_OF(ENUM_MENU(i),ICON_MENU)) != -1) {
			item = gtk_menu_item_new();

			w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
			w1 = gtk_accel_label_new("");

			gtk_container_add(GTK_CONTAINER(w),image(ICON_MENU_FILE_NAME[j]));

			gtk_label_set_use_underline(GTK_LABEL(w1), TRUE);
			gtk_label_set_xalign(GTK_LABEL(w1), 0.0);

			gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(w1), item);

			gtk_box_pack_end(GTK_BOX(w), w1, TRUE, TRUE, 0);

			gtk_container_add(GTK_CONTAINER(item), w);

		} else {
			item = gtk_menu_item_new_with_label("");
		}

		gtk_menu_shell_append(
				GTK_MENU_SHELL(
						subMenu.size() == 0 && bSubMenu ?
								m_menu :
								gtk_menu_item_get_submenu(subMenu.back())),
				item);

		if ((j = INDEX_OF(ENUM_MENU(i),MENU_ACCEL)) != -1) {
			gtk_widget_add_accelerator(item, "activate", m_accelGroup[j],
					ACCEL_KEY[j], GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
		}

		m_menuMap[ENUM_MENU(i)] = item;
		if (bSubMenu) {
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), gtk_menu_new());
			subMenu.push_back(GTK_MENU_ITEM(item));
		} else {
			g_signal_connect(item, "activate", G_CALLBACK (menu_activate),
					GP(i));
		}

		i++;
	}

	fclose(f);

	loadAndUpdateCurrentLanguage();
	//update menu enables/disables, after language[] is filled
	setComboIndex(COMBOBOX_DICTIONARY, dictionary);
	setDictionary();

	loadCSS();

	connectEntrySignals(SEARCH_ENTRY_ID);
	connectEntrySignals(FILTER_ENTRY_ID);

	for (i = 0; i < SIZEI(m_searchButton); i++) {
		g_signal_connect(m_searchButton[i], "clicked",
				G_CALLBACK(button_clicked), NULL);
	}

	g_signal_connect(m_combo[COMBOBOX_SORT_ORDER], "changed",
			G_CALLBACK(combo_changed), gpointer(COMBOBOX_SORT_ORDER));
	//after setComboIndex() & setDictionary()
	g_signal_connect(m_combo[COMBOBOX_DICTIONARY], "changed",
			G_CALLBACK(combo_changed), gpointer(COMBOBOX_DICTIONARY));
	g_signal_connect(m_widget, "destroy", G_CALLBACK (destroy_window), NULL);

#if WINDOW_SIZE_TYPE==0
	const int height = 680;
	gtk_widget_set_size_request(m_widget, 16 * height / 10, height); //notebook resolution 1366x768 40pixels low pane+title
#endif

	gtk_widget_show_all(m_widget);

#if WINDOW_SIZE_TYPE==1 || WINDOW_SIZE_TYPE==2
	//after gtk_widget_show_all
#if WINDOW_SIZE_TYPE==1
	const int height=720;//notebook resolution 1366x768 40pixels low pane, height - full height of window with title
	const int width=16*height/9;//full hd - 1280x720
#else
	//for screenshots on site
	const int width=780;
	const int height=10*width/16;
#endif

	RECT rect;
	HWND h=GetActiveWindow();
	assert(h!=0 && "GetActiveWindow()!=NULL");
	rect.left=rect.top=rect.right=rect.bottom=0;
	AdjustWindowRect(&rect,GetWindowLong(h,GWL_STYLE),FALSE);
	gtk_widget_set_size_request(m_widget,width-(rect.right-rect.left),height-(rect.bottom-rect.top));
#endif

	m_newVersion.start(VERSION_FILE_URL, WORDS_VERSION,
			new_version_message);
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
			gtk_widget_grab_focus(m_text);
			buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text));
			gtk_text_buffer_get_start_iter(buf, &start);
			gtk_text_buffer_get_end_iter(buf, &end);
			gtk_text_buffer_select_range(buf, &start, &end);
		}

		if (menu != MENU_EDIT_SELECT_ALL) {
			clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text));
			if (gtk_text_buffer_get_selection_bounds(buf, &start, &end)) {
				text = gtk_text_buffer_get_text(buf, &start, &end, TRUE); //utf8
				gtk_clipboard_set_text(clipboard, text, -1);
				/* TODO
				 * 4.3 if select search something and then select MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD
				 * then next string gives warning don't know why (bug? not fixed)
				 */
				gtk_clipboard_store(clipboard); //available for other applications
				g_free(text);
			}
		}
		break;

	case MENU_LOAD_ENGLISH_DICTIONARY:
	case MENU_LOAD_RUSSIAN_DICTIONARY:
		setComboIndex(COMBOBOX_DICTIONARY,
				menu == MENU_LOAD_ENGLISH_DICTIONARY ? 0 : 1);
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
		 * earlier user click MENU_REGULAR_EXPRESSIONS, then click MENU_ABOUT and m_menuClick=MENU_ABOUT
		 * and if later make some changes for regular expression entry  got exception because m_menuClick=MENU_ABOUT
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
		/* call only if was last search option, because MENU_ENGLISH_LANGUAGE || MENU_RUSSIAN_LANGUAGE
		 * can be clicked first
		 */
		if (m_menuClick != MENU_SEARCH) {
			setHelperPanel();
			routine();
		}
	}
}

void Frame::destroy() {
	//const std::string CONFIG_TAGS[]={"version","language","dictionary"};
	WRITE_CONFIG(CONFIG_TAGS, WORDS_VERSION,
			getShortLanguageString(m_languageIndex) ,
			getShortLanguageString(getDictionaryIndex()) );

	stopThread();
	gtk_main_quit();
}

void Frame::setDictionary() {
	int i = getDictionaryIndex();
	gtk_widget_set_sensitive(m_menuMap[MENU_LOAD_ENGLISH_DICTIONARY], i != 0);
	gtk_widget_set_sensitive(m_menuMap[MENU_LOAD_RUSSIAN_DICTIONARY], i != 1);

	/* additions 4.3
	 * need to make new search for every option
	 * function setDictionary() can be called several times before any search option so use m_menuClick
	 * as last search option m_menuClick==MENU_SIZE means no last search
	 * m_menuClick changes only for search operations see clickMenu() function
	 * also stopThreadAndNewRoutine() does entry highlight
	 */
	if (m_menuClick != MENU_SEARCH) {
		stopThreadAndNewRoutine();
	}
}

void Frame::aboutDialog() {
	unsigned i, j;
	std::string s;
	GtkWidget *box, *hbox, *dialog, *label;
	char *markup;
	const char *p;

	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), getMenuLabel(MENU_ABOUT).c_str());
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

	const int size = 170;
	GdkPixbuf *pi = gdk_pixbuf_new_from_file_at_size(
			getImagePath("word256.png").c_str(), size, size, 0);
	gtk_container_add(GTK_CONTAINER(hbox), gtk_image_new_from_pixbuf(pi));
	g_object_unref(pi);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	ENUM_STRING sid[] = { PROGRAM, AUTHOR, COPYRIGHT, HOMEPAGE_STRING,
			HOMEPAGE_ONLINE_STRING, STRING_SIZE /*build info*/, EXECUTABLE_FILE_SIZE
	};
	ENUM_STRING id;
	for (i = 0; i < SIZE(sid); i++) {
		id = sid[i];
		if (id == PROGRAM) {
			s=m_programVersion;
		}
		else if (id == STRING_SIZE) {
			for (j = 0; j < SIZE(MONTH); j++) {
				if (strncasecmp(__DATE__, MONTH[j], 3) == 0) {
					break;
				}
			}
			assert(j<SIZE(MONTH));
			//make longer string
			//__DATE__="Dec 15 2016" we need "15 December 2016"
			s =
					format(
							"build %.*s %s %s %s, gcc version %d.%d.%d, gtk version %d.%d.%d ",
							2, __DATE__ + 4, MONTH[j], __DATE__ + 7, __TIME__,
							__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__,
							GTK_MAJOR_VERSION, GTK_MINOR_VERSION,
							GTK_MICRO_VERSION);
		}
		else {
			s = m_language[id];

			if (id == AUTHOR) {
				s += " " + m_language[EMAIL_STRING] + " " + MAIL;
			} else if (id == HOMEPAGE_STRING || id == HOMEPAGE_ONLINE_STRING) {
				s += " ";
				s += id == HOMEPAGE_STRING ? HOMEPAGE : HOMEPAGE_ONLINE;
			} else if (id == COPYRIGHT) {
				j = s.find('(');
				if (j != std::string::npos) {
					s = s.substr(0, j + 1) + "\u00A9" + s.substr(j + 2);
				}
			}else if(id == EXECUTABLE_FILE_SIZE){
				s+=" "+toString(getApplicationFileSize(),',');
			}
		}

		if (id == HOMEPAGE_STRING || id == HOMEPAGE_ONLINE_STRING) {
			p = id == HOMEPAGE_STRING ? HOMEPAGE : HOMEPAGE_ONLINE;
			label = gtk_label_new("");
			markup = g_markup_printf_escaped("%s <a href=\"%s,%s\">\%s,%s</a>",
					m_language[id].c_str(), p,
					LANGUAGE[m_languageIndex].c_str(), p,
					LANGUAGE[m_languageIndex].c_str());
			gtk_label_set_markup(GTK_LABEL(label), markup);
			g_free(markup);
			g_signal_connect(label, "activate-link", G_CALLBACK(label_clicked),
					gpointer(p));

			/*
			 //Extra spaces I cann't remove them
			 label= gtk_link_button_new (format("%s",HOMEPAGE).c_str());
			 g_signal_connect(label, "activate-link",G_CALLBACK(label_clicked), NULL );
			 //g_object_set (G_OBJECT (label),"image-spacing", 10,NULL);
			 addClass(label,"url");
			 //css file .url{padding:0px;margin:0px;border:0px;}
			 */
		} else {
#ifndef NDEBUG
			//output that NDEBUG is not defined
			if(i==0){
				s+=" DEBUG VERSION";
			}
#endif
			label = createLabel(s);
		}

		gtk_widget_set_halign(label, GTK_ALIGN_START);
		addClass(label, "aboutlabel");

		add(box,label);//stretch vertically
		//gtk_container_add(GTK_CONTAINER(box), label);
	}

	gtk_container_add(GTK_CONTAINER(hbox), box);
	gtk_container_add(
			GTK_CONTAINER(gtk_dialog_get_content_area (GTK_DIALOG(dialog))),
			hbox);

	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_widget));

	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void Frame::routine() {
	startJob(true);

	if (prepare()) {
		startThread(thread);
	} else {			//wrapper to call endJob() if prepare() returns false
		endJob();
	}
}

void Frame::setHelperPanel() {
	int i;
	GtkWidget *w;
	gchar *p;
	std::string s;

	clearHelper();

	//set label
	if ((i = INDEX_OF(m_menuClick,HELPER_MENU )) != -1) {//for some of menu items only needs clear helper panel
		w = gtk_label_new("");
		gtk_container_add(GTK_CONTAINER(m_helperUp), w);
		gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_FILL);
		gtk_label_set_line_wrap(GTK_LABEL(w), TRUE);
		gtk_label_set_max_width_chars(GTK_LABEL(w), 40);

		s = replaceAll(m_language[HELPER_STRING[i]], "<br>", "\n");
		p = g_markup_printf_escaped(s.c_str());
		gtk_label_set_markup(GTK_LABEL(w), p);
		g_free(p);

	}

	//add entry with template
	if ((i = INDEX_OF(m_menuClick,TEMPLATE_MENU)) != -1) {
		addEntryLineToHelper(i);
	}

	switch (m_menuClick) {
	case MENU_ANAGRAM:
		addComboLineToHelper(LENGTH, 2, MAX_ANAGRAM_LENGTH, 6, CHARACTERS);
		break;

	case MENU_PANGRAM:
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN);
		add(w, m_language[MINIMUM] + " " + m_language[DIFFERENT_CHARACTERS]);
		add(w, createTextCombo(COMBOBOX_HELPER0, 10, MAX_PANGRAM_LENGTH, 5));
		gtk_container_add(GTK_CONTAINER(m_helperUp), w);
		break;

	case MENU_TEMPLATE:
		//i've changed first selection (since version 4.0) from 2 to 0 for description & combobox agreement
		addComboToHelper(TEMPLATE1, TEMPLATE3, 0);
		break;

	case MENU_REGULAR_EXPRESSIONS:
		addComboLineToHelper(NUMBER_OF_MATCHES, 1, 10, 0, STRING_SIZE);
		break;

	case MENU_MODIFICATION:
		m_check = gtk_check_button_new_with_label(
				m_language[EVERY_MODIFICATION_CHANGES_WORD].c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_check), TRUE);
		gtk_container_add(GTK_CONTAINER(m_helperUp), m_check);
		g_signal_connect(m_check, "toggled", G_CALLBACK(check_changed), NULL);
		break;

	case MENU_SIMPLE_WORD_SEQUENCE:
		addComboLineToHelper(SEQUENCE, 8, MAX_WORD_SEQUENCE_LENGTH, 0,
				CHARACTERS);
		break;

	case MENU_DOUBLE_WORD_SEQUENCE:
		addComboLineToHelper(LENGTH, 2, MAX_DOUBLE_WORD_SEQUENCE_LENGTH, 2,
				CHARACTERS);
		break;

	case MENU_CHARACTER_SEQUENCE:
		addComboToHelper(SEARCH_IN_ANY_PLACE_OF_WORD, SEARCH_IN_END_OF_WORD, 0);
		break;

	case MENU_CONSONANT_VOWEL_SEQUENCE:
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN);
		assert(VOWELS+1==CONSONANTS);
		add(w,
				createTextCombo(COMBOBOX_HELPER0, SEARCH_IN_ANY_PLACE_OF_WORD,
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

		//otherwise nothing to do
	default:
		;
	}

	gtk_widget_show_all(m_helperUp);
}

/**
 * Note function can be called from thread
 */
void Frame::sortFilterAndUpdateResults() {
	sortFilterResults();
	gdk_threads_add_idle(end_job, NULL);
}

void Frame::loadAndUpdateCurrentLanguage() {
	loadLanguage();

	gtk_widget_set_sensitive(m_menuMap[MENU_ENGLISH_LANGUAGE],
			m_languageIndex != 0);
	gtk_widget_set_sensitive(m_menuMap[MENU_RUSSIAN_LANGUAGE],
			m_languageIndex != 1);

	gtk_label_set_text(GTK_LABEL(m_searchLabel), m_language[SEARCH].c_str());
	gtk_label_set_text(GTK_LABEL(m_currentDictionary),
			m_language[DICTIONARY].c_str());

	gtk_window_set_title(GTK_WINDOW(m_widget), m_language[PROGRAM].c_str());

	gtk_frame_set_label(GTK_FRAME(m_filterFrame),m_language[RESULTS_FILTER].c_str());


	refillCombo(COMBOBOX_SORT,SORT_BY_ALPHABET,NUMBER_OF_SORTS);
	refillCombo(COMBOBOX_FILTER,FOUND,2);
}

GtkWidget* Frame::createTextCombo(ENUM_COMBOBOX e, VString v, int active) {
	assert(e!=COMBOBOX_SIZE);

	GtkWidget *w = m_combo[e] = gtk_combo_box_text_new();
	for (auto & a:v) {
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(w), a.c_str());
	}

	setComboIndex(e, active);
	g_signal_connect(w, "changed", G_CALLBACK(combo_changed), gpointer(e));
	return w;
}

GtkWidget* Frame::createTextCombo(ENUM_COMBOBOX e){
	return createTextCombo(e,{},-1);
}

GtkWidget* Frame::createTextCombo(ENUM_COMBOBOX e, int from, int to,
		int active) {
	VString v;
	int i;
	for (i = from; i <= to; i++) {
		v.push_back(std::to_string(i));
	}
	return createTextCombo(e, v, active);
}

GtkWidget* Frame::createTextCombo(ENUM_COMBOBOX e, ENUM_STRING from,
		ENUM_STRING to, int active) {
	VString v;
	int i;
	for (i = from; i <= to; i++) {
		v.push_back(m_language[i]);
	}
	return createTextCombo(e, v, active);
}

GtkWidget* Frame::createEntry(int i) {
	GtkWidget *w = m_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(w), localeToUtf8(getTemplate(i)).c_str());
	connectEntrySignals(0);
	return w;
}

void Frame::addEntryLineToHelper(int i) {
	GtkWidget *w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_container_add(GTK_CONTAINER(w), createLabel(SEARCH));
	add(w, createEntry(i));
	gtk_container_add(GTK_CONTAINER(m_helperUp), w);
}

void Frame::addComboLineToHelper(ENUM_STRING id, int from, int to, int active,
		ENUM_STRING eid) {
	addComboLineToHelper(id, from, to, active,
			m_language[id] + " " + m_language[FROM], m_language[TO],
			eid == STRING_SIZE ? "" : m_language[eid]);
}

void Frame::addComboLineToHelper(ENUM_STRING id, int from, int to, int active,
		std::string s1, std::string s2, std::string s3) {
	int i;
	GtkWidget *w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN);
	for (i = 0; i < 2; i++) {
		add(w, i == 0 ? s1 : s2);
		add(w, createTextCombo(HELPER_COMBOBOX[i], from, to, active));
	}
	if (!s3.empty()) {
		add(w, s3);
	}
	gtk_container_add(GTK_CONTAINER(m_helperUp), w);
}

void Frame::addComboToHelper(ENUM_STRING from, ENUM_STRING to, int active) {
	gtk_container_add(GTK_CONTAINER(m_helperUp),
			createTextCombo(COMBOBOX_HELPER0, from, to, active));
}

void Frame::comboChanged(ENUM_COMBOBOX e) {
	assert(e != COMBOBOX_SIZE);
	updateComboValue(e);

	if (oneOf(e, COMBOBOX_SORT, COMBOBOX_SORT_ORDER, COMBOBOX_DICTIONARY,
			COMBOBOX_FILTER)) {
		waitThread();
		if (e == COMBOBOX_DICTIONARY) {
			setDictionary();
		} else {
			sortOrFilterChanged();
		}
	}
	else {
		stopThread();
		if ((e == COMBOBOX_HELPER0 || e == COMBOBOX_HELPER1)
				&& ONE_OF(m_menuClick,MENU_ADJUST_COMBO )) {
			if (getComboIndex(COMBOBOX_HELPER0)
					> getComboIndex(COMBOBOX_HELPER1)) {
				lockSignals();
				setComboIndex(
						e == COMBOBOX_HELPER0 ?
								COMBOBOX_HELPER1 : COMBOBOX_HELPER0,
						getComboIndex(e));
				unlockSignals();
			}
		}
		//for COMBOBOX_HELPER0-2
		routine();
	}
}

void Frame::entryChanged(int entryIndex) {
	if (entryIndex == SEARCH_ENTRY_ID) {
		waitThread();
		m_tagIndex = 0;
		updateTags();
	}
	else if(entryIndex == FILTER_ENTRY_ID){
		if(prepare()){
			sortOrFilterChanged();
		}
	}
	else {
		stopThreadAndNewRoutine();
	}
}

void Frame::updateTags() {
	GtkTextIter first, scroll;
	GtkTextIter start, end;
	std::string s;
	const gchar *text;

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_remove_tag_by_name(buffer, markTag, &start, &end);
	gtk_text_buffer_remove_tag_by_name(buffer, activeTag, &start, &end);

	s = gtk_entry_get_text(GTK_ENTRY(m_searchEntry));
	s = utf8ToLowerCase(s);
	text = s.c_str();
	if (strlen(text) == 0) {
		gtk_label_set_text(GTK_LABEL(m_searchTagLabel), "");
		return;
	}

	gtk_text_buffer_get_start_iter(buffer, &first);
	for (m_tags = 0;
			gtk_text_iter_forward_search(&first, text,
					GtkTextSearchFlags(
							GTK_TEXT_SEARCH_TEXT_ONLY
									| GTK_TEXT_SEARCH_VISIBLE_ONLY), &start,
					&end, NULL); m_tags++) {

		if (m_tags == m_tagIndex) {
			scroll = start;
		}
		gtk_text_buffer_apply_tag_by_name(buffer,
				m_tags == m_tagIndex ? activeTag : markTag, &start, &end);
		gint offset = gtk_text_iter_get_offset(&end);
		gtk_text_buffer_get_iter_at_offset(buffer, &first, offset);
	}

	if (m_tags > 0) {
		gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(m_text), &scroll, 0.0, true,
				.5, .5);
	}
	gtk_label_set_text(GTK_LABEL(m_searchTagLabel),
			format("%d/%d", m_tags == 0 ? 0 : m_tagIndex + 1, m_tags).c_str());

}
void Frame::createImageCombo(ENUM_COMBOBOX e) {
	int i;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	const char *image[] = { "ascending.png", "descending.png", "en.gif",
			"ru.gif" };
	const int j = e == COMBOBOX_SORT_ORDER ? 0 : 2;

	GtkListStore *gls = gtk_list_store_new(1, GDK_TYPE_PIXBUF);
	for (i = 0; i < 2; i++) {
		gtk_list_store_append(gls, &iter);
		gtk_list_store_set(gls, &iter, 0, pixbuf(image[j + i]), -1);
	}
	m_combo[e] = gtk_combo_box_new_with_model(GTK_TREE_MODEL(gls));
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_combo[e]), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(m_combo[e]), renderer,
			"pixbuf", 0, NULL);
}

void Frame::clickButton(GtkWidget *button) {
	waitThread();
	if (m_tags == 0 || m_tags == 1) {
		return;
	}

	int i;
	for (i = 0; i < SIZEI(m_searchButton); i++) {
		if (button == m_searchButton[i]) {
			break;
		}
	}
	assert(i!=SIZEI(m_searchButton));

	if (i == 0) {
		m_tagIndex++;
	} else {
		m_tagIndex += m_tags - 1;
	}

	m_tagIndex %= m_tags;
	updateTags();
}

void Frame::clearHelper() {
	GList *children, *iter;
	children = gtk_container_get_children(GTK_CONTAINER(m_helperUp));
	for (iter = children; iter != NULL; iter = g_list_next(iter)) {
		gtk_widget_destroy(GTK_WIDGET(iter->data));
	}
	g_list_free(children);
}

void Frame::setMenuLabel(ENUM_MENU e, std::string const &text) {
	GtkWidget *w = m_menuMap[e];
	if (ONE_OF(e,ICON_MENU)) {
		w = gtk_bin_get_child(GTK_BIN(w));
		GList *list = gtk_container_get_children(GTK_CONTAINER(w));
		assert(g_list_length (list)==2);

		gtk_label_set_label(GTK_LABEL(g_list_nth(list, 1)->data), text.c_str());
	} else {
		gtk_menu_item_set_label(GTK_MENU_ITEM(w), text.c_str());
	}
}

std::string Frame::getMenuLabel(ENUM_MENU e) {
	GtkWidget *w = m_menuMap[e];
	if (ONE_OF(e,ICON_MENU)) {
		w = gtk_bin_get_child(GTK_BIN(w));
		GList *list = gtk_container_get_children(GTK_CONTAINER(w));
		assert(g_list_length (list)==2);
		return gtk_label_get_label(GTK_LABEL(g_list_nth(list, 1)->data));
	} else {
		return gtk_menu_item_get_label(GTK_MENU_ITEM(w));
	}
}

void Frame::proceedThread() {
	if (run()) {
		return;
	}
	sortFilterAndUpdateResults();
}

void Frame::startJob(bool clearResult) {
	if (clearResult) {
		m_result.clear();
	}
	m_begin = clock();
	m_out = "";

	//For long jobs show status & view. Long jobs when whole dictionary is added to m_result
	setStatus(m_language[SEARCH] + "...");
	updateTextView();
}

void Frame::endJob() {
	setStatus(getStatusString());

	updateTextView();

	//update tags if user searched something
	m_tagIndex = 0;
	updateTags();

}

/**
 * if thread runs stop it
 */
void Frame::stopThread() {
	g_mutex_lock(&m_mutex);
	waitThread();
	g_mutex_unlock(&m_mutex);
}

bool Frame::userBreakThread() {
	//Sleep(1);//to slowdown check user break

	if (g_mutex_trylock(&m_mutex)) {
		g_mutex_unlock(&m_mutex);
		return false;
	} else {
		m_result.clear();
		m_out = "";
		return true;
	}

}

/**
 * if thread is run, wait while finish
 */
void Frame::waitThread() {
	if (m_thread) {
		/* g_thread_join cann't be called several times. see documentation
		 * on second call program hang out
		 * stopThread() can be called several times, and stopThread() calls waitThread()
		 * so use m_thread variable as indicator
		 */
		g_thread_join(m_thread);
		m_thread = 0;
		//update status & GtkTextBuffer
		endJob();
	}
}

void Frame::updateComboValue(ENUM_COMBOBOX e) {
	assert(e!=COMBOBOX_SIZE);
	int v = -1;
	if (GTK_IS_COMBO_BOX_TEXT(m_combo[e])) {
		char *p = gtk_combo_box_text_get_active_text(
				GTK_COMBO_BOX_TEXT(m_combo[e]));
		if (p && stringToInt(p,v)) {
			assert(v>=0);
		}
	}
	if (v == -1) {
		v = getComboIndex(e);
		assert(v!=-1);
	}
	m_comboValue[e] = v;
}

bool Frame::prepare() {

	if (m_menuClick == MENU_MODIFICATION) {
		m_checkValue = gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(m_check))==TRUE;
	}

	bool hasEntry = INDEX_OF(m_menuClick,TEMPLATE_MENU) != -1;

	if (hasEntry) {
		//should encode to locale string at first to get valid length
		m_entryText = utf8ToLocale(gtk_entry_get_text(GTK_ENTRY(m_entry)));
	}

	m_filterText=utf8ToLocale(gtk_entry_get_text(GTK_ENTRY(m_filterEntry)));
	if(!setCheckFilterRegex()){
		addClass(m_filterEntry, CERROR);			//red font
		return false;
	}

	if (!WordsBase::prepare()) {
		if (hasEntry) {
			addClass(m_entry, CERROR);			//red font
		}
		return false;
	}

	if (hasEntry) {
		removeClass(m_entry, CERROR);			//normal font, all valid
	}

	removeClass(m_filterEntry, CERROR);			//normal font, all valid

	return true;
}

void Frame::connectEntrySignals(int index) {
	GtkWidget *w;
	switch(index){
	case SEARCH_ENTRY_ID:
		w = m_searchEntry;
		break;

	case FILTER_ENTRY_ID:
		w = m_filterEntry;
		break;

	default:
		w=m_entry;
	}
	g_signal_connect_after(G_OBJECT(w), "insert-text", G_CALLBACK(entry_insert),
			GP(index));
	g_signal_connect_after(G_OBJECT(w), "delete-text", G_CALLBACK(entry_delete),
			GP(index));
	g_signal_connect_after(G_OBJECT(w), "focus-in-event",
			G_CALLBACK(entry_focus_in), GP(index));
	g_signal_connect_after(G_OBJECT(w), "focus-out-event",
			G_CALLBACK(entry_focus_out), GP(index));
}

void Frame::entryFocusChanged(bool in) {
	if (in) {
		removeAccelerators();
	} else {
		addAccelerators();
	}
}

void Frame::removeAccelerators() {
	int i;
	for (i = 0; i < MENU_ACCEL_SIZE; i++) {
		gtk_window_remove_accel_group(GTK_WINDOW(m_widget), m_accelGroup[i]);
	}
}

/**
 * Note even if program use accelerators and combobox is active then hotkeys don't work
 */
void Frame::addAccelerators() {
	for (int i = 0; i < MENU_ACCEL_SIZE; i++) {
		gtk_window_add_accel_group(GTK_WINDOW(m_widget), m_accelGroup[i]);
	}
}

void Frame::sortOrFilterChanged(){
	/* m_result is empty nothing to do, may be we have dictionary statistics or something like this
	 * calling of sortFilterAndUpdateResults() clear m_result
	 */
	if (!m_result.empty()) {
		/* sortAndUpdateResults() could take a long time so use thread
		 */
		startJob(false);
		startThread(sort_filter_thread);
	}
}

void Frame::refillCombo(ENUM_COMBOBOX e,ENUM_STRING first,int length) {
	int i,j = getComboIndex(e);
	if(j==-1){//was empty combo
		if(e==COMBOBOX_SORT){
			//sort by length descendant
			j=1;
		}
		else{
			//regex filter "match" option - default filter
			j=0;
		}
	}
	lockSignals();
	auto c=GTK_COMBO_BOX_TEXT(m_combo[e]);
	gtk_combo_box_text_remove_all(c);
	for (i = 0; i < length; i++) {
		gtk_combo_box_text_append_text(c, m_language[first + i].c_str());
	}
	setComboIndex(e, j);
	unlockSignals();
}

void Frame::newVersionMessage() {
	std::string s=m_language[NEW_VERSION_MESSAGE]+"\n"+m_newVersion.m_message;
	GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(m_widget), GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, s.c_str());

	gtk_window_set_title(GTK_WINDOW(d), m_programVersion.c_str());
	gint result = gtk_dialog_run(GTK_DIALOG(d));
	gtk_widget_destroy(d);

	if (result == GTK_RESPONSE_YES) {
		openURL(DOWNLOAD_URL);
	}
}
