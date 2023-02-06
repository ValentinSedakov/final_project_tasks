#include "scaner.c"
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

struct scroll_win_user
{
    WINDOW *win;
    struct user_list *head;
    int step;
    bool is_active;
};

struct scroll_win_table
{
    WINDOW *win;
    struct file_name_list *head;
    int step;
    bool is_active;
};

struct win_progress_bar
{
    WINDOW *win;
    struct queue_loaders *queue;
};

struct scroll_win_user *scroll_win_user(int NLINES, int NCOLS, int y, int x)
{
    struct scroll_win_user *scroll_win_user = (struct scroll_win_user *)malloc(sizeof(struct scroll_win_user));
    scroll_win_user->win = newwin(NLINES, NCOLS, y, x);
    scroll_win_user->step = 0;
    scroll_win_user->head = NULL;
    scroll_win_user->is_active = false;
    wclear(scroll_win_user->win);
	wrefresh(scroll_win_user->win);
    keypad(scroll_win_user->win, TRUE);
    wbkgdset(scroll_win_user->win, COLOR_PAIR(1));
    return scroll_win_user;
}

struct scroll_win_table *scroll_win_table(int NLINES, int NCOLS, int y, int x)
{
    struct scroll_win_table *scroll_win_table = (struct scroll_win_table *)malloc(sizeof(struct scroll_win_table));
    scroll_win_table->win = newwin(NLINES, NCOLS, y, x);
    scroll_win_table->step = 0;
    scroll_win_table->head = NULL;
    scroll_win_table->is_active = false;
    wclear(scroll_win_table->win);
	wrefresh(scroll_win_table->win);
    keypad(scroll_win_table->win, TRUE);
    wbkgdset(scroll_win_table->win, COLOR_PAIR(1));
    return scroll_win_table;
}

struct win_progress_bar *win_progress_bar(int NLINES, int NCOLS, int y, int x)
{
    struct win_progress_bar *win_progress_bar = (struct win_progress_bar *)malloc(sizeof(struct win_progress_bar));
    win_progress_bar->win = newwin(NLINES, NCOLS, y, x);
    win_progress_bar->queue = NULL;
    wclear(win_progress_bar->win);
	wrefresh(win_progress_bar->win);
    keypad(win_progress_bar->win, TRUE);
    wbkgdset(win_progress_bar->win, COLOR_PAIR(1));
    return win_progress_bar;
}

void draw_win_user(struct scroll_win_user *win)
{
    wattron(win->win,COLOR_PAIR(2));
    wclear(win->win);
    wmove(win->win,0,0);
    if (win->head != NULL)
    {
        int count = size_user_list(&(win->head));

        for (int i = 0; i < count; i++)
        {
            if (i==win->step&&win->is_active)
            {
                wattron(win->win,COLOR_PAIR(2));
            } else {
                wattron(win->win,COLOR_PAIR(1));
            }
            wprintw(win->win,"%s\n",get_user(&(win->head),i)->ip);
        }
    }

    
    
    wrefresh(win->win);
    

}
void draw_win_table(struct scroll_win_table *win)
{
    wattron(win->win,COLOR_PAIR(2));
    wclear(win->win);
    wmove(win->win,0,0);
    
    if (win->head != NULL)
    {
        wprintw(win->win,"owner:%s\n",win->head->file_name.owner.ip);
        int count = size_file_name_list(&(win->head));
        for (int i = 0; i < count; i++)
        {
            if (i==win->step && win->is_active)
            {
                wattron(win->win,COLOR_PAIR(2));
            } else {
                wattron(win->win,COLOR_PAIR(1));
            }
            wprintw(win->win,"%s\n",get_file_name(&(win->head),i)->file_name);
        }
    }
    
    
    wrefresh(win->win);
}
void draw_win_progress_bar(struct win_progress_bar *win)
{
    wattron(win->win,COLOR_PAIR(2));
    wclear(win->win);
    wmove(win->win,0,0);
    if (front_queue_loaders(&(win->queue)) != NULL){

    }
}

int main(int argc, char const *argv[])
{
    WINDOW *temp_win;
    bool work = true;
    int ch = 0;
    struct scaner *scaner = NULL;
    struct table *table = NULL;
    struct user_list *user_list = NULL;
    struct file_name_list *file_name_list = NULL;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    /* инициализация цветовой палитры */
    if (!has_colors())
    {
        endwin();
        printf("\nОшибка! Не поддерживаются цвета\n");
        return 1;
    }
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);

    struct scroll_win_user *win_user = scroll_win_user(getmaxy(stdscr)-4, getmaxx(stdscr)/2, 0, 0);
    win_user->is_active = true;
    start_scan(&scaner);

    struct scroll_win_table *win_table = scroll_win_table(getmaxy(stdscr)-4, getmaxx(stdscr)/2, 0, getmaxx(stdscr)/2+1);

    struct win_progress_bar *win_progress = win_progress_bar(4, getmaxx(stdscr), getmaxy(stdscr)-3, 0);

    halfdelay(3);

    temp_win = win_table->win;
    while (work)
    {
        // провера на текущую работу scaner
        if (scaner != NULL)
        {
            switch (get_user_list(&scaner, &user_list))
            {
            case READY:
                /* code */
                stop_scan(&scaner);
                win_user->head = user_list;
                int i = size_user_list(&user_list);
                if (i <= win_user->step)
                {
                    win_user->step = i - 1;
                }
                break;
            case WORK:

                break;
            case FAIL:
                stop_scan(&scaner);
                break;
            default:
                break;
            }
        }
        // провера на текущую работу table
        if (table != NULL)
        {
            switch (get_file_name_list(&table, &file_name_list))
            {
            case READY:
                stop_table(&table);
                //printf("READY HEAD ---------%p\n", win_table->head);
                //sleep(2);
                win_table->head = file_name_list;
                win_table->step = 0;
                break;
            case WORK:

                break;
            case FAIL:
                stop_table(&table);
                break;
            default:
                break;
            }
        }
        //проверка на готовность первого в очереди лоадера
        //если он готов то закрывает этот лоадер
        struct loader *temp = front_queue_loaders(&(win_progress->queue));
        if (load_ready(&temp) == 100)
        {
            pop_queue_loaders(&(win_progress->queue));
        }

        if (win_user->is_active)
        {
            switch (ch = getch())
            {
            case KEY_F(5):
                win_user->head = NULL;
                start_scan(&scaner);
                break;

            case 9://TAB
            case KEY_RIGHT:
                win_user->is_active = false;
                win_table->is_active = true;
                break;

            case 10://ENTER
                start_table(&table, *get_user(&(win_user->head), win_user->step));
                win_table->win = temp_win;
                break;

            case KEY_DOWN:

                if (size_user_list(&user_list) > (win_user->step) + 1)
                {
                    win_user->step++;
                }
                break;

            case KEY_UP:
                if (0 <= win_user->step - 1)
                {
                    win_user->step--;
                }
                break;

            case KEY_BACKSPACE:
                work = false;
                break;

            default:
               
                break;
            }
        }

        if (win_table->is_active)
        {
            switch (ch = getch())
            {

            case 9: //TAB
            case KEY_LEFT:
                win_user->is_active = true;
                win_table->is_active = false;
                break;

            case 10://ENTER
                struct file_name *file_name = get_file_name(&file_name_list, win_table->step);
                push_queue_loaders(&(win_progress->queue), start_load(*file_name));
                break;
            case KEY_DOWN:
                if (size_file_name_list(&file_name_list) > (win_table->step) + 1)
                {
                    win_table->step++;
                }
                break;

            case KEY_UP:
                if (0 <= win_table->step - 1)
                {
                    win_table->step--;
                }
                break;

            case KEY_BACKSPACE:
                work = false;
                break;

            default:
                
                break;
            }
        }

        draw_win_user(win_user);
        draw_win_table(win_table);
        draw_win_progress_bar(win_progress);
    }
    
    // завершение программы
    delwin(win_table->win);
    delwin(win_user->win);
    delwin(win_progress->win);
    endwin();
    delete_user_list(&user_list);
    delete_file_name_list(&file_name_list);
    delete_queue_loaders(&(win_progress->queue));
    free(win_table);
    free(win_user);
    free(win_progress);
    return 0;
}