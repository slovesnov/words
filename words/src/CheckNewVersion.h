/*
 * CheckNewVersion.h
 *
 *  Created on: 14.02.2018
 *      Author: alexey slovesnov
 */

#ifndef CHECKNEWVERSION_H_
#define CHECKNEWVERSION_H_

#include <string>
#include <gtk/gtk.h>

class CheckNewVersion {
	GtkWindow *m_parent;
	std::string m_title;
	std::string m_versionUrl;
	std::string m_downloadUrl;
	double m_version;
	std::string m_message;
	GThread* m_newVersionThread;
public:
	CheckNewVersion();
	void init(GtkWindow *parent, std::string title, std::string versionUrl, std::string downloadUrl, double version, std::string message);
	~CheckNewVersion();
	void routine();
  void newVesionMessage();
};

#endif /* CHECKNEWVERSION_H_ */
