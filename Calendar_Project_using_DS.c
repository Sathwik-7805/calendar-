#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

struct cal {
    int d, m, y;
    char event[100];
    struct cal *next;
};

void mon();
int disp();
int monthcode(int m);
int centurycode(int c);
int leapyearcode(int l);
int daycode(int d);
int ndinmon(int m, int y);
void adj();
void jmp();
int addevent();
int searchevent(int, int, int);
void dispall();
void sort();
void swapd();
void swape();
void db_init();
void db_close();
void db_add_event(struct cal event);
struct cal db_search_event(int d, int m, int y);
void db_display_events();
void write_to_file(struct cal event);
sqlite3 *db;
struct cal *start = NULL, *temp, *ptr, *ne;
static int date, month, year;
int i, j, k, r, flag = 0, opt, c;
char ch[3], y[] = "yes", n[] = "no", op;

int main() {
    db_init();
    static int z = 1;
    if (z == 1) {
        printf("\n---------------------------------------------");
        printf("\nWELCOME TO THE CALENDAR PROGRAM");
        printf("\n---------------------------------------------");
        FILE *fptr = fopen("demo.txt", "w");
        fputs("       EVENTS \n", fptr);
        fclose(fptr);
    }

    while (1) {
        printf("\n\n\nPlease go through the choice given below:\n");
        printf("\n1.Display Month Calendar");
        printf("\n2.Adjacent Calendar");
        printf("\n3.Jump to date ");
        printf("\n4.Display Year Calendar");
        printf("\n5.Add Event");
        printf("\n6.Search Event");
        printf("\n7.Display All Events");
        printf("\n8.Sort By Dates");
        printf("\n9.Exit ");
        printf("\n\nEnter the Option:");
        scanf("%d", &opt);
        switch (opt) {
            case 1:
                mon();
                printf("\n\nEnter the month:");
                scanf("%d", &month);
                printf("Enter the year:");
                scanf("%d", &year);
                disp();
                break;
            case 2:
                adj();
                break;
            case 3:
                jmp();
                break;
            case 4:
                printf("\nEnter the year:");
                scanf("%d", &year);
                for (month = 1; month <= 12; month++) {
                    disp();
                }
                break;
            case 5:
                addevent();
                break;
            case 6:
                printf("\nEnter the Date:");
                scanf("%d", &date);
                printf("\nEnter the Month:");
                scanf("%d", &month);
                printf("\nEnter the Year:");
                scanf("%d", &year);
                flag = searchevent(date, month, year);
                if (flag == 0 || temp == NULL) {
                    printf("\nNo Event is there on %d / %d / %d", date, month, year);
                } else {
                    printf("\n\n%d /  %d  /  %d ", temp->d, temp->m, temp->y);
                    printf("\n%s", temp->event);
                }
                break;
            case 7:
                dispall();
                break;
            case 8:
                sort();
                break;
            case 9:
                printf("\nExiting program.....\nThank you");
                db_close();
                exit(0);
            default:
                printf("Invalid Choice");
        }
        z++;
    }
    return 0;
}

void db_init() {
    int rc = sqlite3_open("calendar.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }
    const char *sql = "CREATE TABLE IF NOT EXISTS Events ("
                      "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "Day INT,"
                      "Month INT,"
                      "Year INT,"
                      "Event TEXT);";
    char *err_msg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

void db_close() {
    sqlite3_close(db);
}

int addevent() {
    ne = (struct cal *)malloc(sizeof(struct cal));
    printf("\nEnter the Date:");
    scanf("%d", &ne->d);
    printf("\nEnter the Month:");
    scanf("%d", &ne->m);
    printf("\nEnter the Year:");
    scanf("%d", &ne->y);
    printf("\nEnter the Text:");
    scanf(" %[^\n]s", ne->event);
    flag = searchevent(ne->d, ne->m, ne->y);
    if (flag == 1) {
        printf("\nEvent already exists.Do you want to overwrite");
        printf("\nEnter YES or NO:");
        scanf("%s", ch);
        r = strcasecmp(ch, y);
        if (r == 0) {
            strcpy(temp->event, ne->event);
        } else {
            temp = start;
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = ne;
            ne->next = NULL;
            printf("\nEvent Successfully added");
        }
    } else {
        if (start == NULL) {
            ne->next = NULL;
            start = ne;
        } else {
            temp = start;
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = ne;
            ne->next = NULL;
        }
        printf("\nEvent Successfully added");
    }
    db_add_event(*ne);
    write_to_file(*ne);
    return 0;
}

void db_add_event(struct cal event) {
    char *err_msg = 0;
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Events (Day, Month, Year, Event) VALUES (%d, %d, %d, '%s');",
             event.d, event.m, event.y, event.event);
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

int searchevent(int date, int month, int year) {
    flag = 0;
    if (start == NULL) {
        return flag;
    } else {
        temp = start;
        while (temp != NULL) {
            if (temp->d == date && temp->m == month && temp->y == year) {
                flag = 1;
                break;
            }
            temp = temp->next;
        }
        return flag;
    }
}

struct cal db_search_event(int d, int m, int y) {
    struct cal event = {0};
    char *err_msg = 0;
    sqlite3_stmt *stmt;
    char sql[128];
    snprintf(sql, sizeof(sql), "SELECT Event FROM Events WHERE Day = %d AND Month = %d AND Year = %d;", d, m, y);
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            event.d = d;
            event.m = m;
            event.y = y;
            strcpy(event.event, (const char *)sqlite3_column_text(stmt, 0));
        }
    }
    sqlite3_finalize(stmt);
    return event;
}

void dispall() {
    db_display_events();
    if (start == NULL)
        printf("\nNo Events");
    else {
        printf("\n\n\nDisplaying Events...");
        temp = start;
        while (temp != NULL) {
            printf("\n\n%d /  %d  /  %d ", temp->d, temp->m, temp->y);
            printf("\n%s", temp->event);
            temp = temp->next;
        }
    }
}

void db_display_events() {
    char *err_msg = 0;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT Day, Month, Year, Event FROM Events;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    } else {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("\n\n%d / %d / %d: %s\n",
                   sqlite3_column_int(stmt, 0),
                   sqlite3_column_int(stmt, 1),
                   sqlite3_column_int(stmt, 2),
                   sqlite3_column_text(stmt, 3));
        }
    }
    sqlite3_finalize(stmt);
}

void write_to_file(struct cal event) {
    FILE *fptr = fopen("demo.txt", "a");
    if (fptr != NULL) {
        fprintf(fptr, "\n\n%d /  %d  /  %d \n%s\n", event.d, event.m, event.y, event.event);
        fclose(fptr);
    } else {
        printf("Can't open the file");
    }
}

void mon() {
    printf("\n_____________________________________________");
    printf("\n\nPlease go through the choice given below:\n");
    printf("\n1.JANUARY");
    printf("\n2.FEBRUARY");
    printf("\n3.MARCH");
    printf("\n4.APRIL");
    printf("\n5.MAY");
    printf("\n6.JUNE");
    printf("\n7.JULY");
    printf("\n8.AUGUST");
    printf("\n9.SEPTEMBER");
    printf("\n10.OCTOBER");
    printf("\n11.NOVEMBER");
    printf("\n12.DECEMBER");
}

int disp() {
    int sp, p, gap, date = 1, mc, cc, lyc, yc, cent, dc, tot;
    char week[7][7] = {"Sat   ", "Sun   ", "Mon   ", "Tue   ", "Wed   ", "Thr   ", "Fri   "};
    printf("\n\n\n________________________________________");
    printf("\n\n~~The calendar of ");
    switch (month) {
        case 1: printf("JANUARY"); break;
        case 2: printf("FEBRUARY"); break;
        case 3: printf("MARCH"); break;
        case 4: printf("APRIL"); break;
        case 5: printf("MAY"); break;
        case 6: printf("JUNE"); break;
        case 7: printf("JULY"); break;
        case 8: printf("AUGUST"); break;
        case 9: printf("SEPTEMBER"); break;
        case 10: printf("OCTOBER"); break;
        case 11: printf("NOVEMBER"); break;
        case 12: printf("DECEMBER"); break;
    }
    printf(" %d~~", year);
    printf("\n________________________________________\n\n\n");
    k = ndinmon(month, year);
    for (i = 0; i <= 6; i++) {
        for (j = 0; j <= 6; j++) {
            if (i == 0) {
                printf("%s", week[j]);
            } else if (i == 1) {
                yc = year % 100;
                cent = year / 100;
                mc = monthcode(month);
                cc = centurycode(cent);
                lyc = leapyearcode(yc);
                if (date == 1) {
                    tot = 1 + mc + cc + yc + lyc;
                    dc = daycode(tot);
                    sp = dc;
                    if ((year % 4 == 0 && month == 2) || (month == 1 && year % 4 == 0))
                        p = sp - 1;
                    else
                        p = sp;
                    for (gap = 0; gap < p; gap++)
                        printf("      ");
                    for (j = p; j <= 6; j++) {
                        if (date < 10)
                            printf("%d     ", date++);
                        else
                            printf(" %d    ", date++);
                    }
                } else {
                    if (date < 10)
                        printf("%d     ", date++);
                    else
                        printf("%d    ", date++);
                    if (date == k + 1)
                        goto nxt;
                }
            } else {
                if (date < 10)
                    printf("%d     ", date++);
                else
                    printf("%d    ", date++);
                if (date == k + 1)
                    goto nxt;
            }
        }
        printf("\n");
    }
nxt:
    printf("\n\n-----------------------------------------");
}

void adj() {
    printf("\nInput > for forward and < for Backward");
    printf("\nEnter the choice:");
    scanf(" %c", &op);
    if (op == '>') {
        if (month <= 11)
            month += 1;
        else if (month == 12) {
            month = 1;
            year += 1;
        }
        disp();
    } else if (op == '<') {
        if (month > 1)
            month -= 1;
        else if (month == 1) {
            month = 12;
            year -= 1;
        }
        disp();
    } else {
        printf("\nInvalid Character");
    }
}

void jmp() {
    mon();
    printf("\n\nEnter the month:");
    scanf("%d", &month);
    printf("Enter the year:");
    scanf("%d", &year);
    disp();
}

void sort() {
    if (start == NULL || start->next == NULL) {
        printf("\nNo events to sort");
    } else {
        for (temp = start; temp->next != NULL; temp = temp->next) {
            for (ptr = temp->next; ptr != NULL; ptr = ptr->next) {
                if (temp->y > ptr->y) {
                    swapd();
                    swape();
                } else if (temp->y == ptr->y) {
                    if (temp->m > ptr->m) {
                        swapd();
                        swape();
                    } else if (temp->m == ptr->m) {
                        if (temp->d >= ptr->d) {
                            swapd();
                            swape();
                        }
                    }
                }
            }
        }
    }
}

void swapd() {
    int c;
    c = temp->d;
    temp->d = ptr->d;
    ptr->d = c;
    c = temp->m;
    temp->m = ptr->m;
    ptr->m = c;
    c = temp->y;
    temp->y = ptr->y;
    ptr->y = c;
}

void swape() {
    char t[100];
    strcpy(t, temp->event);
    strcpy(temp->event, ptr->event);
    strcpy(ptr->event, t);
}

int monthcode(int m) {
    int mc;
    switch (m) {
        case 1: mc = 1; break;
        case 2: mc = 4; break;
        case 3: mc = 4; break;
        case 4: mc = 0; break;
        case 5: mc = 2; break;
        case 6: mc = 5; break;
        case 7: mc = 0; break;
        case 8: mc = 3; break;
        case 9: mc = 6; break;
        case 10: mc = 1; break;
        case 11: mc = 4; break;
        case 12: mc = 6; break;
    }
    return mc;
}

int centurycode(int c) {
    int r, rcc;
    r = c % 4;
    switch (r) {
        case 0: rcc = 6; break;
        case 1: rcc = 4; break;
        case 2: rcc = 2; break;
        case 3: rcc = 0; break;
    }
    return rcc;
}

int leapyearcode(int l) {
    int nl;
    nl = l / 4;
    return nl;
}

int daycode(int d) {
    d = d % 7;
    return d;
}

int ndinmon(int m, int y) {
    int nd;
    if (m == 1) nd = 31;
    else if (m == 2) {
        if ((y % 400 == 0) || (y % 4 == 0 && y % 100 != 0)) nd = 29;
        else nd = 28;
    } else if (m == 3) nd = 31;
    else if (m == 4) nd = 30;
    else if (m == 5) nd = 31;
    else if (m == 6) nd = 30;
    else if (m == 7) nd = 31;
    else if (m == 8) nd = 31;
    else if (m == 9) nd = 30;
    else if (m == 10) nd = 31;
    else if (m == 11) nd = 30;
    else nd = 31;
    return nd;
}
