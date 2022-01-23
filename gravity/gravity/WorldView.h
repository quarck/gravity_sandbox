#pragma once 

#include <memory>

#define _USE_MATH_DEFINES
#include <math.h>

#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */

#include "glText.h"

#include "World.h"

#include "Props.h"

namespace gravity
{
	struct WorldViewDetails
	{
		int numActiveThreads;
		int64_t secondsEmulated;
		double timeRate; // seconds of emulated time per second of a real time 
		bool showDetailedcontrols;
		bool paused;

		WorldViewDetails(int nThr, bool p) 
			: numActiveThreads{ nThr }
			, secondsEmulated{ 0 }
			, timeRate{ 0 }
			, showDetailedcontrols { false }
			, paused { p }
		{

		}
	};

	template <typename TWorld, typename TBody>
    class WorldView
    {
		static constexpr uint32_t LABELS_BACKGROUND = 0xff000000;
		static constexpr uint32_t CONTROLS_LABEL_FOREGROUND = 0xff0f0f7f;
		static constexpr uint32_t RUGA_KOLORO = 0xff0f0fdf;
		static constexpr uint32_t VERDA_KOLORO = 0xff006f00u;
		static constexpr uint32_t CFG_CLR_FOREGROUND = 0xff9f004fu;

		static constexpr double LOCATION_SCALE{ ONE_A_U * 2 / props::ViewPortWidth }; 		
		
		TWorld& _world;

		glText::Label _controlsLabel{ LABELS_BACKGROUND, CONTROLS_LABEL_FOREGROUND, "<?> - help" };

		glText::Label _controlsLabelDetailed{
			LABELS_BACKGROUND,
			{
				std::pair(RUGA_KOLORO, "<S> - Save,  <L> - Load" /*", <R> - Reset" */),
				std::pair(RUGA_KOLORO, "<T> - toggle recording"),
				std::pair(RUGA_KOLORO, "< or > - cycle focused object"),
				std::pair(RUGA_KOLORO, "<?> - help ON/OFF, <SPACE> - (un)pause, <esc> - quit"),
			}
		};

		glText::Label _iterAndCfgLabel{ LABELS_BACKGROUND, VERDA_KOLORO, "_TMP_" };

		glText::Label _pausedLabel{ LABELS_BACKGROUND, RUGA_KOLORO, "<< PAUSED >>" };

		//Color _foodColor{ 192, 64, 64 };

		int _zoom{ 4 * 256 };
		int _current_focused_object{ 0 };
		
    public:

        WorldView(TWorld& world)
            : _world(world)
        {
            Random rnd = Random();
		}


		void zoomIn()
		{
			if (_zoom > 1)
				_zoom /= 2;
		}

		void zoomOut()
		{
			_zoom *= 2;
		}

		void zoomReset()
		{
			_zoom = 4 * 256;
		}

		void resetFocusObject()
		{
			_current_focused_object = 0;
		}

		void focusNextObject()
		{
			_current_focused_object++; // will deal with over/under-flows when updating view 
		}
		void focusPrevObject()
		{
			_current_focused_object--; // will deal with over/under-flows when updating view 
		}

		void PrintControls(const WorldViewDetails& details) noexcept
		{
			glPushMatrix();

			glPixelZoom(1.f, 1.f);

			((details.showDetailedcontrols || details.paused) ? _controlsLabelDetailed : _controlsLabel)
				.DrawAt(-1.0, -0.99);

			if (details.paused)
				_pausedLabel.DrawAt(-0.2, 0);

			glPopMatrix();
		}

		void PrintStats(const WorldViewDetails& details) noexcept
		{
			glPushMatrix();

			glPixelZoom(1.f, 1.f);

			std::ostringstream ostr;

			int64_t seconds_emulated = details.secondsEmulated;
			int64_t years = seconds_emulated / (365 * 24 * 3600);
			seconds_emulated %= (365 * 24 * 3600);
			int64_t days = seconds_emulated / (24 * 3600);

			ostr << "Y: " << years << ", D:" << days;
			
			ostr << ", R: " << static_cast<int64_t>(details.timeRate / 1000) << "k:1";

			std::ostringstream rcfg;
			rcfg << "#THR: " << details.numActiveThreads;

			_iterAndCfgLabel.Update(
				LABELS_BACKGROUND,
				{ 
					std::pair(VERDA_KOLORO, ostr.str()),
					std::pair(CFG_CLR_FOREGROUND, ""), // rcfg.str()),
				});
			_iterAndCfgLabel.DrawAt(-1.0, 0.94);

			glPopMatrix();
		}

		template <typename TBody>
		void DrawBody(const TBody& body, double vpx, double vpy)
		{
			glPushMatrix();

			auto scaled_x{ (body.location.value.x()-vpx) / LOCATION_SCALE / _zoom * 256 };
			auto scaled_y{ (body.location.value.y()-vpy) / LOCATION_SCALE / _zoom * 256 };

			glTranslatef(scaled_x, scaled_y, 0.0);
			//glRotatef(
			//    static_cast<float>(_body.Rotation / M_PI * 180.0 - 90.0),
			//    0.0f, 0.0f, 1.0f);

			glBegin(GL_TRIANGLES);


			if (body.temperature > 6000.0)
			{
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			else if (body.temperature > 2500.0)
			{
				glColor3f(1.0f, 1.0f, 0.0f);
			}
			else if (body.temperature > 700.0)
			{
				glColor3f(1.0f, 0.0f, 0.0f);
			}
			else
			{
				glColor3f(0.2f, 0.4f, 1.0f);
			}

			float factor = 1000.0;
			if (_zoom > 128)
			{
				factor = 1000.0;
			}
			else if (_zoom == 128)
			{
				factor = 128.0 * 500.0 / _zoom;
			}

			float radius = static_cast<float>(body.radius / LOCATION_SCALE  * factor * 256 / _zoom);

			radius = std::max<float>(1.0f, std::log(radius));

			int idx = 0;
			float prev_x{ radius };
			float prev_y{ 0.0f };

			for (float angle = M_PI / 10.0; angle < 2 * M_PI + M_PI / 10; angle += M_PI / 10)
			{
				glIndexi(++idx); glVertex2f(0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(prev_x, prev_y);
				prev_x = radius * std::cos(angle);
				prev_y = radius * std::sin(angle);
				glIndexi(++idx); glVertex2f(prev_x, prev_y);
			}

			glEnd();

			glPopMatrix();
		}

        void UpdateFrom(
			TWorld& world,
			const WorldViewDetails& details, 
			bool hideControlsAndStats
		)  noexcept
        {
            glPushMatrix();

            // BG BEGIN
            glBegin(GL_TRIANGLES);

            glColor3f(0.0f, 0.0f, 0.0f);

            glIndexi(1); glVertex2f(1.0f, 1.0f);
            glIndexi(2); glVertex2f(-1.0f, 1.0f);
            glIndexi(3); glVertex2f(-1.0f, -1.0f);

            glIndexi(4); glVertex2f(1.0f, 1.0f);
            glIndexi(5); glVertex2f(1.0f, -1.0f);
            glIndexi(6); glVertex2f(-1.0f, -1.0f);

            glEnd();
            // BG END
			if (!hideControlsAndStats)
			{
				PrintControls(details);
			}
			PrintStats(details);

            glScalef(
                static_cast<GLfloat>(2.0 / gravity::props::ViewPortWidth),
                static_cast<GLfloat>(2.0 / gravity::props::ViewPortHeight),
                1.0f);

           // glTranslatef(-gravity::props::ViewPortWidth / 2.0f, -gravity::props::ViewPortHeight / 2.0f, 0.0);

			const auto& objects = world.get_objects();
			if (objects.size() > 0)
			{
				while (_current_focused_object < 0)
					_current_focused_object += objects.size();

				_current_focused_object = _current_focused_object % objects.size();

				double vpx{ objects[_current_focused_object].location.value.x() };
				double vpy{ objects[_current_focused_object].location.value.y() };

				for (auto& b : objects)
				{
					DrawBody(b, vpx, vpy);
				}
			}

            //for (auto& food : _world->GetFoods())
            //{
            //    if (food.EnergyValue < 0.01)
            //        continue;

            //    glPushMatrix();
            //    glTranslatef(food.LocationX, food.LocationY, 0.0);
            //    //glRotatef(cell->Rotation, 0.0f, 0.0f, 1.0f);

            //    glBegin(GL_TRIANGLES);

            //    _foodColor.GlApply();

            //    float halfdiameter = static_cast<float>(std::sqrt(food.EnergyValue) * 5.0 / 1.5);

            //    int idx = 0;
            //    glIndexi(++idx); glVertex3f(0.0f, halfdiameter, 0.0f);
            //    glIndexi(++idx); glVertex3f(halfdiameter / 1.5f, 0.0f, 0.0f);
            //    glIndexi(++idx); glVertex3f(-halfdiameter / 1.5f, 0.0f, 0.0f);

            //    glIndexi(++idx); glVertex3f(0.0f, -halfdiameter, 0.0f);
            //    glIndexi(++idx); glVertex3f(halfdiameter / 1.5f, 0.0f, 0.0f);
            //    glIndexi(++idx); glVertex3f(-halfdiameter / 1.5f, 0.0f, 0.0f);


            //    glIndexi(++idx); glVertex3f(halfdiameter, 0.0f, 0.0f);
            //    glIndexi(++idx); glVertex3f(0.0f, halfdiameter / 1.5f, 0.0f);
            //    glIndexi(++idx); glVertex3f(0.0f, -halfdiameter / 1.5f, 0.0f);

            //    glIndexi(++idx); glVertex3f(-halfdiameter, 0.0f, 0.0f);
            //    glIndexi(++idx); glVertex3f(0.0f, halfdiameter / 1.5f, 0.0f);
            //    glIndexi(++idx); glVertex3f(0.0f, -halfdiameter / 1.5f, 0.0f);

            //    glEnd();

            //    glPopMatrix();
            //}

            glPopMatrix();
		}
    };
}
