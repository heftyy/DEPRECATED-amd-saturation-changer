#pragma once

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

struct settings
{
public:
	std::string process_name;
	int normal_saturation;
	int process_saturation;
	int normal_brightness;
	int process_brightness;
	int normal_contrast;
	int process_contrast;
	int logical_display_id;
	
	void load(const std::string &filename)
	{
		// Create an empty property tree object
		using boost::property_tree::ptree;
		ptree pt;

		// Load the XML file into the property tree. If reading fails
		// (cannot open file, parse error), an exception is thrown.;
		read_ini(filename, pt);

		// Get the filename and store it in the m_file variable.
		// Note that we construct the path to the value by separating
		// the individual keys with dots. If dots appear in the keys,
		// a path type with a different separator can be used.
		// If the debug.filename key is not found, an exception is thrown.)
		process_name = pt.get<std::string>("process_name");

		// Get the debug level and store it in the m_level variable.
		// This is another version of the get method: if the value is
		// not found, the default value (specified by the second
		// parameter) is returned instead. The type of the value
		// extracted is determined by the type of the second parameter,
		// so we can simply write get(...) instead of get<int>(...).
		normal_saturation = pt.get("normal_saturation", 100);
		process_saturation = pt.get("process_saturation", 200);
		normal_brightness = pt.get("normal_brightness", 0);
		process_brightness = pt.get("process_brightness", 25);
		normal_contrast = pt.get("normal_contrast", 100);
		process_contrast = pt.get("process_contrast", 125);

		logical_display_id = pt.get("logical_display_id", 0);
	}

	void save(const std::string &filename)
	{
		// Create an empty property tree object
		using boost::property_tree::ptree;
		ptree pt;

		// Put log filename in property tree
		pt.put("debug.filename", process_name);

		// Put debug level in property tree
		pt.put("debug.level", normal_saturation);
		pt.put("debug.level", process_saturation);
		pt.put("debug.level", normal_brightness);
		pt.put("debug.level", process_brightness);
		pt.put("debug.level", normal_contrast);
		pt.put("debug.level", process_contrast);
		pt.put("debug.level", logical_display_id);

		// Write the property tree to the XML file.
		write_ini(filename, pt);
	}
};