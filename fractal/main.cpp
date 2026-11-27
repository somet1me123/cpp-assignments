#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>

class DragonWidget : public Fl_Widget
{
    int depth = 12;

    void draw_dragon(double x1, double y1, double x2, double y2, int d)
    {
        if (d == 0)
        {
            fl_line(static_cast<int>(x1), static_cast<int>(y1),
                    static_cast<int>(x2), static_cast<int>(y2));
            return;
        }
        double xn = (x1 + x2) / 2.0 + (y2 - y1) / 2.0;
        double yn = (y1 + y2) / 2.0 - (x2 - x1) / 2.0;

        draw_dragon(x1, y1, xn, yn, d - 1);
        draw_dragon(x2, y2, xn, yn, d - 1);
    }

public:
    DragonWidget(int X, int Y, int W, int H, int d = 14)
        : Fl_Widget(X, Y, W, H), depth(d) {}

    void set_depth(int d)
    {
        depth = d;
        redraw();
    }

    void draw() override
    {
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        fl_color(fl_rgb_color(0, 220, 180));

        double x1 = x() + w() * 0.25;
        double y1 = y() + h() * 0.65;
        double x2 = x() + w() * 0.80;
        double y2 = y() + h() * 0.65;

        draw_dragon(x1, y1, x2, y2, depth);
    }
};

int main()
{
    Fl_Double_Window *win = new Fl_Double_Window(800, 640, "Фрактал Дракон (FLTK)");
    DragonWidget *dragon = new DragonWidget(0, 0, 800, 600, 12);

    Fl_Value_Slider *slider = new Fl_Value_Slider(80, 608, 700, 24, "Глубина:");
    slider->type(FL_HOR_NICE_SLIDER);
    slider->align(FL_ALIGN_LEFT);
    slider->bounds(0, 17);
    slider->step(1);
    slider->value(12);

    slider->callback([](Fl_Widget *w, void *data)
                     {
        auto* s = static_cast<Fl_Value_Slider*>(w);
        auto* d = static_cast<DragonWidget*>(data);
        d->set_depth(static_cast<int>(s->value())); }, dragon);

    win->end();
    win->resizable(dragon);
    win->show();
    return Fl::run();
}