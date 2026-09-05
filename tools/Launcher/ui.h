#pragma once

#include "updater.h"

#include <imgui.h>
#include <string>
#include <vector>

class UI
{
public:
	UI();
	void Render(int width, int height);

private:
	// Fixed x offsets from the window's left edge, shared by every row so the
	// labels, tick marks, browse buttons and action buttons line up in columns
	// instead of landing wherever the preceding text happens to end.
	struct Columns
	{
		float inputWidth;
		float label;
		float icon;
		float browse;
		float buttonWidth;
	};

	static Columns CalcColumns();
	bool SelectFile(std::string& str, const std::string& label, const Columns& col, const std::vector<std::string>& ext, const std::vector<std::string>& filters, const std::string& tip);
	bool SelectFolder(std::string& str, const std::string& label, const Columns& col, const std::string& tip);

private:
	Updater m_updater;
	std::string m_version = "None";
	std::string m_biosPath;
	std::string m_gamePath;
	std::string m_username;
	std::string m_status;
};
