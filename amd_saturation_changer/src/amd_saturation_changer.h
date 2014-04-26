#pragma once

#include <iostream>

#include "settings.h"
#include "interruptable_thread.h"
#include "amd_saturation_controller.h"
#include "process_manager.h"

class amd_saturation_changer
{
public:
	std::unique_ptr<amd_saturation_controller> saturation_controller;
	std::unique_ptr<struct settings> settings;
	std::unique_ptr<process_manager> process;
	std::unique_ptr<interruptible_thread> check_for_process;

	amd_saturation_changer()
	{
		saturation_controller = std::unique_ptr<amd_saturation_controller>(new amd_saturation_controller());

		settings = std::unique_ptr<struct settings>(new struct settings());
		load_settings();

		std::cout << "Found and loaded settings file" << std::endl;

		check_for_process = std::unique_ptr<interruptible_thread>(new interruptible_thread());
		process = std::unique_ptr<class process_manager>(new class process_manager());

		std::cout << "Waiting for processs " << settings->process_name << std::endl;
	}

	~amd_saturation_changer()
	{
		stop();
		std::cout << "Shutting down" << std::endl;
	}

	void start()
	{
		check_for_process->start_thread([this]()
		{
			int current_saturation = saturation_controller->get_setting(settings->logical_display_id, ADL_DISPLAY_COLOR_SATURATION);
			int current_brightness = saturation_controller->get_setting(settings->logical_display_id, ADL_DISPLAY_COLOR_BRIGHTNESS);
			int current_contrast = saturation_controller->get_setting(settings->logical_display_id, ADL_DISPLAY_COLOR_CONTRAST);
			while (1)
			{
				check_for_process->check_for_interrupt();

				if (process->is_process_running(settings->process_name) && current_saturation != settings->process_saturation)
				{
					std::cout << "Found process " << settings->process_name << ", changing saturation" << std::endl;
					current_saturation = saturation_controller->set_setting(settings->process_saturation, settings->logical_display_id, ADL_DISPLAY_COLOR_SATURATION);
					current_brightness = saturation_controller->set_setting(settings->process_brightness, settings->logical_display_id, ADL_DISPLAY_COLOR_BRIGHTNESS);
					current_contrast = saturation_controller->set_setting(settings->process_contrast, settings->logical_display_id, ADL_DISPLAY_COLOR_CONTRAST);
				}
				else if (!process->is_process_running(settings->process_name) && current_saturation != settings->normal_saturation)
				{
					std::cout << "Didn't find process " << settings->process_name << ", changing saturation" << std::endl;
					current_saturation = saturation_controller->set_setting(settings->normal_saturation, settings->logical_display_id, ADL_DISPLAY_COLOR_SATURATION);
					current_brightness = saturation_controller->set_setting(settings->normal_brightness, settings->logical_display_id, ADL_DISPLAY_COLOR_BRIGHTNESS);
					current_contrast = saturation_controller->set_setting(settings->normal_contrast, settings->logical_display_id, ADL_DISPLAY_COLOR_CONTRAST);
				}
				std::chrono::milliseconds sleep_duration(500);
				std::this_thread::sleep_for(sleep_duration);
			}
		});
	}

	void set_default_settings() 
	{
		saturation_controller->set_setting(settings->normal_saturation, settings->logical_display_id, ADL_DISPLAY_COLOR_SATURATION);
		saturation_controller->set_setting(settings->normal_brightness, settings->logical_display_id, ADL_DISPLAY_COLOR_BRIGHTNESS);
		saturation_controller->set_setting(settings->normal_contrast, settings->logical_display_id, ADL_DISPLAY_COLOR_CONTRAST);
	}

	void stop()
	{
		check_for_process->stop();
		set_default_settings();
	}

	void load_settings()
	{
		try
		{
			settings->load("settings.ini");
		}
		catch (std::runtime_error)
		{
			std::cerr << "Couldn't read settings file" << std::endl;
			return;
		}
	}

};