#pragma once 

#define _CRT_SECURE_NO_WARNINGS 1

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

#include <Commdlg.h>
#include <Windows.h> // file dialogs 
#include <Shlobj.h>
#include <Shlobj_core.h>

#include "World.h"
#include "WorldView.h"
#include "RuntimeConfig.h"
#include "IImageLogger.h"
#include "PngLogger.h"

namespace gravity
{
    class MainController
    {
	public:
		using TWorld = World;
		using TObject = mass_body;
		using TWorldView = WorldView<TWorld, TObject>;

	private:
		RuntimeConfig& config;

        TWorld world;
        std::mutex worldLock;
        TWorldView _worldView;


		WorldViewDetails viewDetails;

		std::unique_ptr<IImageLogger> _imageLogger;

		//int iterationPerSeconds{ 0 };
		//long currentStep{ 0 };

        std::thread calcThread;

        std::atomic_bool terminate{ false };

        std::atomic_bool uiNeedsUpdate{ false };
        std::atomic_bool appPaused{ true };

		std::atomic_bool recording{ false };

        HDC hDC;				/* device context */
        HPALETTE hPalette{ 0 };			/* custom palette (if needed) */

        HWND hWND;

		int _vpWidth{ 1 };
		int _vpHeight{ 1 };

    public:

        MainController(RuntimeConfig& cfg)
            : config(cfg)
			, viewDetails { 1, true }
			, world{ }
			, _worldView{ world }
        {
            //string documents = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
            //string workingFolder = $"{documents}\\Neurolution\\{DateTime.Now:yyyy-MM-dd-HH-mm}";

            //world = std::make_shared<TWorld>(
            //    std::string(""),
            //    config.GetNumWorkerThreads());

            //_worldView = std::make_shared<TWorldView>(world);
        }

        ~MainController()
        {
            terminate = true;
            if (calcThread.joinable())
                calcThread.join();
        }

        void Start()
        {
            calcThread = std::thread(&MainController::CalcThread, this);
        }

        void Stop()
        {
            terminate = true;
        }

        void CalcThread()
        {
            auto lastUIUpdate = std::chrono::high_resolution_clock::now();
			int64_t last_update_at{ 0 };

            while(!terminate)
            {
				while (appPaused && !terminate)
				{
					::Sleep(100);
					uiNeedsUpdate = true;
					::SendMessage(hWND, WM_USER, 0, 0);
					while (uiNeedsUpdate && !terminate)
					{
						// Keep yeild-ing the thread while UI thread is doing the painting job, 
						// this is to avoid the white lock situation
						std::this_thread::yield();
					}
				}

				if (world.current_iteration() % 1024 == 0)
				{
					auto now = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double> sinceLastUpdate = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastUIUpdate);

					if (sinceLastUpdate.count() > 1.0 / 30.0)
					{						
						viewDetails.timeRate = gravity::TIME_DELTA * (world.current_iteration() - last_update_at) / static_cast<double>(sinceLastUpdate.count()); // seconds per second 
						lastUIUpdate = now;
						last_update_at = world.current_iteration();

						uiNeedsUpdate = true;
						::SendMessage(hWND, WM_USER, 0, 0);
						while (uiNeedsUpdate && !terminate)
						{
							// Keep yeild-ing the thread while UI thread is doing the painting job, 
							// this is to avoid the white lock situation
							std::this_thread::yield();
						}
					}
                }

                std::lock_guard<std::mutex> l(worldLock);
                world.iterate();
				viewDetails.secondsEmulated = static_cast<int64_t>(static_cast<double>(world.current_iteration()) * gravity::TIME_DELTA);
            }
        }

		void onViewportResize(int width, int height)
		{
			_vpWidth = width;
			_vpHeight = height;
			if (_imageLogger)
				_imageLogger->onViewportResize(width, height);
		}

		void onToggleScreenRecording()
		{
			if (!recording && !_imageLogger)
			{
				WCHAR file[MAX_PATH];

				BROWSEINFO bi;
				ZeroMemory(&bi, sizeof(bi));
				bi.hwndOwner = hWND;
				bi.lpszTitle = &file[0];
				bi.ulFlags = 0; // check it 


				LPITEMIDLIST lpItem = SHBrowseForFolder(&bi);
				if (lpItem != NULL)
				{
					SHGetPathFromIDList(lpItem, file);

					char mbsFolder[MAX_PATH * 4];
					size_t nc = ::wcstombs(mbsFolder, file, MAX_PATH * 4 - 1);
					if (nc > 0 && nc < MAX_PATH * 4)
					{
						_imageLogger = std::make_unique<PngLogger>(mbsFolder);
						_imageLogger->onViewportResize(_vpWidth, _vpHeight);
					}
				}
			}

			recording = !recording && _imageLogger;
		}

		void onZoomInView()
		{
			_worldView.zoomIn();
		}

		void onZoomOutView()
		{
			_worldView.zoomOut();
		}

		void onZoomResetView()
		{
			_worldView.zoomReset();
		}

		void OnKeyboard(WPARAM wParam)
		{
			switch (wParam)
			{
			case 27:			/* ESC key */
				onExit();
				break;
			case ' ':
				appPaused = !appPaused;
				break;

			case '?':
				viewDetails.showDetailedcontrols = !viewDetails.showDetailedcontrols;
				__faststorefence();
				break;

			case 'S': case 's': 
				onSave();
				break;

			case 'L': case 'l': 
				onLoad();
				break;

			case 'r': case 'R': 
				onResetWorld();
				break;

			case 'f': case 'F': 
				onToggleFreezePredators();
				break;

			case 'b': case 'B': 
				onBrainwashPredators();
				break;

			case 'g': case 'G': 
				onRecoverHamsters();
				break;

			case 't': case 'T': 
				onToggleScreenRecording();
				break;

			case '+': case '=':
				onZoomInView();
				break;

			case '-': case '_':
				onZoomOutView();
				break;

			case '0': 
				onZoomResetView();
				break;
			}
		}

        void DrawWorld()
        {
            std::lock_guard<std::mutex> l(worldLock);
			viewDetails.paused = appPaused;
            _worldView.UpdateFrom(world, viewDetails, recording);
            uiNeedsUpdate = false;

			if (_imageLogger && recording && !appPaused)
			{
				_imageLogger->onNewFrame();
			}
        }

        bool IsUINeedsUpdate() const { return uiNeedsUpdate; }
        void ClearUINeedsUpdate() { uiNeedsUpdate = false; }

        bool IsAppPaused() const { return appPaused; }
        void SetAppIsPaused(bool val) { appPaused = val; }

        bool IsTerminating() const { return terminate; }


        const HWND& GetHWND() const { return hWND; }
        void SetHWND(HWND hwnd) 
        { 
            hWND = hwnd; 
            SetHDC(GetDC(hWND));
        }

        const HDC& GetHDC() const { return hDC; }
        void SetHDC(HDC hdc) { hDC = hdc; }

        const HPALETTE& GetHPalette() const { return hPalette; }
        void SetHPalette(HPALETTE hp) { hPalette = hp; }

	private:

		void onExit()
		{
			int ret = ::MessageBox(hWND, L"Save before exiting?", L"Caption", MB_YESNOCANCEL);

			if (ret == IDYES)
			{
				if (!onSave())
				{
					return;
				}
			}
			else if (ret == IDCANCEL)
			{
				return;
			}

			Stop();
			PostQuitMessage(0);
		}

		bool onSave()
		{
			bool ret = false;

			WCHAR file[MAX_PATH];
			char mbsFile[MAX_PATH * 4];

			auto now = std::chrono::system_clock::now();
			auto in_time_t = std::chrono::system_clock::to_time_t(now);

//
			std::stringstream ssFilename;
			tm tm;
			localtime_s(&tm, &in_time_t);
			ssFilename << std::put_time(&tm, "%Y%m%d_%H%M%S.nn");
			::mbstowcs(file, ssFilename.str().c_str(), MAX_PATH - 1);

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);

			ofn.hwndOwner = hWND;
			ofn.lpstrFilter = L"Gravity (*.gra)\0*.gra\0";
			ofn.lpstrFile = &file[0];
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = L"nn";

			if (::GetSaveFileName(&ofn))
			{
				size_t nc = ::wcstombs(mbsFile, file, MAX_PATH * 4 - 1);
				if (nc > 0 && nc < MAX_PATH * 4)
				{
					std::lock_guard<std::mutex> l(worldLock);
					std::ofstream file(mbsFile, std::ofstream::out | std::ofstream::binary);
					world.save_to(file);
					ret = true;
				}
			}

			return ret;
		}

		void onLoad()
		{
			WCHAR file[MAX_PATH] = L"";
			char mbsFile[MAX_PATH * 4];

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);

			ofn.hwndOwner = hWND;
			ofn.lpstrFilter = L"Gravity (*.gra)\0*.gra\0";
			ofn.lpstrFile = &file[0];
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = L"nn";

			if (::GetOpenFileName(&ofn))
			{
				size_t nc = ::wcstombs(mbsFile, file, MAX_PATH * 4 - 1);
				if (nc > 0 && nc < MAX_PATH * 4)
				{
					std::lock_guard<std::mutex> l(worldLock);
					std::ifstream file(mbsFile, std::ifstream::in | std::ifstream::binary);
					world.load_from(file);
				}
			}
		}

		void onResetWorld()
		{

		}

		void onToggleFreezePredators()
		{

		}

		void onBrainwashPredators()
		{

		}

		void onRecoverHamsters()
		{

		}
    };
}
