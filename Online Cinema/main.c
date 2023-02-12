//
//  main.c
//  CinemaOnline_NEW
//
//  Created by Danil Degtyarev on 16.12.2022.
//
#include "TextColor.h"
#include <dirent.h>
#include <termios.h>
#include <ctype.h>
#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

typedef struct Film {
    char title[96];
    int year;
    char countries[64];
    char genres[64];
    float rating;

    struct Film *prev;
    struct Film *next;
} Film;

typedef struct Films {
    int size;
    Film *current;
} Films;

typedef struct {
    char login[24];
    char password[24];
    char card[20];
    Films *favorites;
    int is_admin;
} User;

// Выбор пункта меню
int input_mode() {
    struct termios tio;

    if (tcgetattr(STDIN_FILENO, &tio) != 0) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    tio.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio) != 0) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    int ch = getchar();

    tio.c_lflag |= ICANON | ECHO;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio) != 0) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    return ch;
}

// Создание кольцевого списка
Films *create_films_ring() {
    Films *films = (Films *)malloc(sizeof(Films));
    films->size = 0;
    films->current = NULL;
    return films;
}

// Добавление нового фильма в список
Film *add_film(Films *ring) {
    Film *film = (Film *)malloc(sizeof(Film));

    if (ring->current == NULL) {
        ring->current = film;
        film->prev = film;
        film->next = film;
    } else {
        film->prev = ring->current->prev;
        film->next = ring->current;
        ring->current->prev->next = film;
        ring->current->prev = film;
        ring->current = film;
    }
    ring->size++;
    return film;
}

// Удаление фильма из списка
void remove_film(Films *films, Film *film)
{
    if (films->current == film && film->next == film) {
        films->current = NULL;
    } else {
        film->prev->next = film->next;
        film->next->prev = film->prev;

        if (films->current == film) films->current = film->next;
    }
    films->size--;
    free(film);
}

// Считываем данные о фильмах из файла
Films *get_films_from_file(const char *filename) {
    FILE *txt = fopen(filename, "r");
    Films *films = create_films_ring();

    fseek(txt, 0, SEEK_END);
    long pos = ftell(txt);
    if (pos == 0) {
        return films;
    }
    fseek(txt, 0, SEEK_SET);

    while (!feof(txt)) {
        Film *film = add_film(films);
        fgets(film->title, sizeof(film->title), txt);
        fscanf(txt, "%d\n", &film->year);
        fgets(film->countries, sizeof(film->countries), txt);
        fgets(film->genres, sizeof(film->genres), txt);
        fscanf(txt, "%f\n", &film->rating);

        strtok(film->title, "\n");
        strtok(film->countries, "\n");
        strtok(film->genres, "\n");
    }
    fclose(txt);
    return films;
}

// Счетчик длины
size_t get_len(const char *string) {
    size_t size = 0;
    char byte;
    for (int i = 0;; i++) {
        byte = string[i];
        if (byte == '\0') {
            return size;
        }
        size += (((byte & 0x80) == 0) || ((byte & 0xC0) == 0xC0));
    }
}

// Вывод линий с информацией о фильме.
void print_line_with_spaces(const char *string, const char position) {
    size_t title_length = get_len(string), title_spaces;
    if (!position) {
        title_spaces = (46 - title_length) / 2;
    } else {
        title_length /= 2;
        if (title_length % 2 == 0) title_length++;
        title_spaces = (23 - title_length) / 2;
    }

    if (position != 2) printf("║");
    for (int j = 0; j < title_spaces; j++) printf(" ");

    text_red(stdout);
    if (!position) printf("%s", string);
    else for (int j = 0; j < title_length; j++) printf("*");
    text_blue(stdout);

    if ((!position && title_length % 2 != 0) || (position && title_length % 2 == 0)) title_spaces++;
    for (int j = 0; j < title_spaces; j++) printf(" ");

    if (position != 1) printf("║");

}

// Вывод карусели с фильмами
void print_cards(Film *film) {
    text_bold(stdout);
    text_blue(stdout);
    printf("                        ╔══════════════════════════════════════════════╗");
    printf("                        \n                        ");
    printf("║                                              ║");
    printf("                        \n                        ");
    printf("║                                              ║");
    printf("                        \n╔═══════════════════════");
    print_line_with_spaces(film->title, 0);
    printf("═══════════════════════╗\n║                       ");
    printf("║                                              ║");
    printf("                       ║\n");
    print_line_with_spaces(film->prev->title, 1);
    print_line_with_spaces(film->genres, 0);
    print_line_with_spaces(film->next->title, 2);
    printf("\n║                       ");
    printf("║                                              ║");
    printf("                       ║\n");
    print_line_with_spaces(film->prev->genres, 1);
    print_line_with_spaces(film->countries, 0);
    print_line_with_spaces(film->next->genres, 2);
    printf("\n║                       ");
    printf("║                                              ║");
    printf("                       ║\n");
    print_line_with_spaces(film->prev->countries, 1);
    printf("║                     %.1f+                     ║", film->rating);
    print_line_with_spaces(film->next->countries, 2);
    printf("\n║                       ");
    printf("║                                              ║");
    printf("                       ║\n║                       ");
    printf("║                                              ║");
    printf("                       ║\n║                       ");
    printf("║                                              ║");
    printf("                       ║\n║          ***          ");
    printf("║                                              ║");
    printf("          ***          ║\n║                       ");
    printf("║                                              ║");
    printf("                       ║\n╚═══════════════════════");
    printf("║                     %d                     ║", film->year);
    printf("═══════════════════════╝\n                        ");
    printf("║                                              ║");
    printf("                        \n                        ");
    printf("║                                              ║");
    printf("                        \n                        ");
    printf("╚══════════════════════════════════════════════╝\n");
    reset_colors(stdout);
}

// Вывод гигачада
void print_gigachad() {
    printf("Пока-пока!\n");
}

// Поиск пользователя в базе данных из файла users.txt
User *get_user() {
    FILE *users_txt = fopen("users.txt", "r+");
    User *user = (User*)malloc(sizeof(User));
    char user_input[24];

    regex_t regex;
    if (regcomp(&regex, "^[[:alpha:][:digit:]]{3,20}$", REG_EXTENDED)) {
        fprintf(stderr, "Не удалось скомпилировать regex-выражение.\n");
        exit(1);
    }

    while (1) {
        printf("Для входа или регистрации введите свой логин (от 3 до 20 символов) >> ");
        scanf("%s", user_input);

        if (regexec(&regex, user_input, 0, NULL, 0) == REG_NOMATCH) {
            printf("Логин должен состоять из латинских букв и цифр. И его длина должна быть в диапазоне 3-20 символов.\n");
            continue;
        }

        while (fgets(user->login, sizeof(user->login), users_txt)) {
            fgets(user->password, sizeof(user->password), users_txt);
            fgets(user->card, sizeof(user->card), users_txt);
            fscanf(users_txt, "%d\n", &user->is_admin);

            strtok(user->login, "\n");
            strtok(user->password, "\n");
            strtok(user->card, "\n");

            if (!strcmp(user->login, user_input)) {
                fclose(users_txt);
                return user;
            }
        }
        free(user);
        user = (User*)malloc(sizeof(User));
        strcpy(user->login, user_input);
        return user;
    }
}

// Авторизация существующего пользователя
char login(User* user) {
    char password[20];
    printf("Вход в аккаунт %s.\nВведите пароль >> ", user->login);
    scanf("%s", password);
    if (!strcmp(user->password, password)) {
        char filename[36] = "favorites/";
        strcat(filename, user->login);
        strcat(filename, ".txt");
        user->favorites = get_films_from_file(filename);

        return 1;
    }
    return 0;
}

// Сохранение данных о пользователе в файл
void save_user(User *user) {
    FILE *users_txt = fopen("users.txt", "a");
    fprintf(users_txt, "%s\n", user->login);
    fprintf(users_txt, "%s\n", user->password);
    fprintf(users_txt, "%s\n", user->card);
    fprintf(users_txt, "%d\n", user->is_admin);
    fclose(users_txt);
}

char check_luhn(const char *card_number) {
    int len = (int) strlen(card_number);
    int number, sum = 0;
    for(int i = 0; i < len; i++) {
        if (!isdigit(card_number[i])) return 0;
        number = card_number[i] - '0';
        if ((i & 1) == 0) {
            number *= 2;
            if(number > 9) number -= 9;
        }
        sum += number;
        if (sum >= 10) sum -= 10;
    }
    if (sum == 0) return 1;
    else return 0;
}

// Создание нового аккаунта для пользователя
void sign_up(User* user) {
    printf("Регистрация аккаунта с логином %s.\n", user->login);
    while (1) {
        printf("Введите пароль для аккаунта (от 6 до 20 символов) >> ");
        scanf("%s", user->password);
        int has_lowercase = 0, has_uppercase = 0, has_number = 0;
        for (int i = 0; i < strlen(user->password); i++) {
            if (islower(user->password[i])) has_lowercase++;
            if (isupper(user->password[i])) has_uppercase++;
            if (isdigit(user->password[i])) has_number++;
        }
        if (has_lowercase && has_uppercase && has_number && strlen(user->password) >= 6) break;
        else
            printf("Пароль должен содержать хотя бы одну латинскую букву в верхнем и нижнем регистре. "
                   "И его длина должна быть в диапазоне от 3 до 20 символов\n");
    }
    printf("Ваш будущий пароль для входа в аккаунта %s: %s\n", user->login, user->password);

    while (1) {
        printf("Введите номер карты без пробелов >> ");
        scanf("%s", user->card);
        if (!check_luhn(user->card)) printf("Карта невалидна.\n");
        else break;
    }

    char filename[36] = "favorites/";
    strcat(filename, user->login);
    strcat(filename, ".txt");
    FILE *file = fopen(filename, "w");
    fclose(file);

    user->favorites = create_films_ring();
    user->is_admin = 0;
    save_user(user);
}

// Авторизация/Регистрация аккаунта
User *auth() {
    printf("Приветствую тебя в онлайн-кинотеатре.\n");
    while (1) {
        User *user = get_user();
        if (!strlen(user->password)) {
            system("clear");
            sign_up(user);
        }
        system("clear");
        if (!login(user)) {
            printf("Введён неверный пароль. Попробуйте войти снова.\n");
            free(user);
            continue;
        }
        return user;
    }
}

// Запись информации о фильме в файл
void write_film(FILE *file, Film *film) {
    fprintf(file, "%s\n", film->title);
    fprintf(file, "%d\n", film->year);
    fprintf(file, "%s\n", film->countries);
    fprintf(file, "%s\n", film->genres);
    fprintf(file, "%.1f\n", film->rating);
}

// Добавить фильм в избранные
Film *add_favorite_film(User *user, Film *film) {
    Film *favorite_film = add_film(user->favorites);
    strcpy(favorite_film->title, film->title);
    favorite_film->year = film->year;
    strcpy(favorite_film->countries, film->countries);
    strcpy(favorite_film->genres, film->genres);
    favorite_film->rating = film->rating;

    char filename[36] = "favorites/";
    strcat(filename, user->login);
    strcat(filename, ".txt");

    FILE *favorites_file = fopen(filename, "a");
    write_film(favorites_file, favorite_film);
    fclose(favorites_file);

    return favorite_film;
}

// Удаление фильма из файла
void remove_film_from_file(Films *films, Film *delete_film, const char *filename) {
    FILE *temp_file = fopen("temp.txt", "a");
    Film *film = films->current;
    do {
        if (strstr(film->title, delete_film->title)) {
            remove_film(films, film);
            break;
        }
        film = film->next;
    } while (film != films->current);

    if (films->current) {
        film = films->current;
        do {
            write_film(temp_file, film);
            film = film->next;
        } while (film != films->current);
    }
    fclose(temp_file);
    rename("temp.txt", filename);
}

// Удаление фильма из избранных
void remove_favorite_film(User *user, Film *film) {
    char filename[36] = "favorites/";
    strcat(filename, user->login);
    strcat(filename, ".txt");
    remove_film_from_file(user->favorites, film, filename);
}

// Удаление фильма из каталога
void remove_film_admin(Films *films, User *user, Film *film) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir("favorites");

    char filename[36];
    Films *favorite_films;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        strcpy(filename, "favorites/");
        strcat(filename, entry->d_name);
        favorite_films = get_films_from_file(filename);
        if (favorite_films->current != NULL) {
            remove_film_from_file(favorite_films, film, filename);
        }
        free(favorite_films);
    }
    closedir(dir);
    remove_favorite_film(user, film);
    remove_film_from_file(films, film, "films.txt");
}

// Вывод подробной информации о фильме
int print_addition_info(Films *films, User *user, Film *film, Film *in_favorites, const char view_type) {
    while (1) {
        system("clear");
        printf("Название: %s\n", film->title);
        printf("Год производства: %d\n", film->year);
        printf("Страны производства: %s\n", film->countries);
        printf("Жанры: %s\n", film->genres);
        printf("Рейтинг: %.1f\n\n", film->rating);
        printf("Q - выход, ");
        if (view_type == 2) {
            printf("R - удалить фильм");
        } else {
            if (in_favorites) printf("R - удалить из избранных.\n");
            else printf("F - добавить в избранное.\n");
        }

        int ch = input_mode();
        if (ch == 'f' && !in_favorites) {
            in_favorites = add_favorite_film(user, film);
        } else if (ch == 'r' && in_favorites && view_type != 2) {
            remove_favorite_film(user, in_favorites);
            in_favorites = NULL;
            if (view_type) return 1;
        } else if (ch == 'r' && view_type == 2) {
            remove_film_admin(films, user, film);
        } else if (ch == 'q') {
            return 0;
        }
    }
}

// Проверка фильма на наличие в списке избранных
Film *check_favorites(Films *favorites, Film *film) {
    Film *favorite_film = favorites->current;
    do {
        if (strstr(favorite_film->title, film->title)) return favorite_film;
        favorite_film = favorite_film->next;
    }
    while (favorites->current != favorite_film);

    return NULL;
}

// Навигация внутри карусели
char show_films(Films *films, User *user, const char view_type) {
    Film *film = films->current;
    while (films->current != NULL) {
        system("clear");
        print_cards(film);
        printf("                             Переход между фильмами на кнопки A и "
               "D\n");
        printf("                      Подробная информация - M, ");

        Film *in_favorites = NULL;
        if (view_type != 2) {
            if (user->favorites->size)
                in_favorites = check_favorites(user->favorites, film);
            if (!in_favorites)
                printf("Добавить в избранные - F\n");
            else
                printf("Удалить из избранных - R\n");
        } else {
            printf("Удалить из каталога - R");
        }

        int ch = input_mode();
        if (ch == 'a') {
            film = film->prev;
        } else if (ch == 'd') {
            film = film->next;
        } else if (ch == 'f' && !in_favorites && view_type != 2) {
            add_favorite_film(user, film);
        } else if (ch == 'r' && in_favorites && view_type != 2) {
            if (view_type == 1) film = film->next;
            remove_favorite_film(user, in_favorites);
        } else if (ch == 'r' && view_type == 2) {
            Film *temp = film->next;
            remove_film_admin(films, user, film);
            film = temp;
        } else if (ch == 'm') {
            if (print_addition_info(films, user, film, in_favorites, view_type)) film = film->next;
        } else if (ch == 'q') {
            return 0;
        }
    }
    return 1;
}

// Проверка пользователя на существование в системе с определенным логином
char check_field(const char *field) {
    FILE *users_txt = fopen("users.txt", "r");
    User *user = (User *)malloc(sizeof(User));
    while (fgets(user->login, sizeof(user->login), users_txt)) {
        fgets(user->password, sizeof(user->password), users_txt);
        fgets(user->card, sizeof(user->card), users_txt);
        fscanf(users_txt, "%d\n", &user->is_admin);

        strtok(user->login, "\n");
        strtok(user->password, "\n");
        strtok(user->card, "\n");

        if (!strcmp(user->login, field)) {
            free(user);
            fclose(users_txt);
            return 1;
        }
    }

    free(user);
    fclose(users_txt);
    return 0;
}

// Редактирование строк в файле
void edit_field(const char *login, const char *field_name, const char *field) {
    FILE *users_txt = fopen("users.txt", "r");
    FILE *users_temp = fopen("temp.txt", "w");

    User *temp_user = (User *)malloc(sizeof(User));
    while (fgets(temp_user->login, sizeof(temp_user->login), users_txt)) {
        fgets(temp_user->password, sizeof(temp_user->password), users_txt);
        fgets(temp_user->card, sizeof(temp_user->card), users_txt);
        fscanf(users_txt, "%d\n", &temp_user->is_admin);

        strtok(temp_user->login, "\n");
        strtok(temp_user->password, "\n");
        strtok(temp_user->card, "\n");

        if (!strcmp(field_name, "login") && !strcmp(temp_user->login, login)) fprintf(users_temp, "%s\n", field);
        else fprintf(users_temp, "%s\n", temp_user->login);
        if (!strcmp(field_name, "password") && !strcmp(temp_user->login, login)) fprintf(users_temp, "%s\n", field);
        else fprintf(users_temp, "%s\n", temp_user->password);
        if (!strcmp(field_name, "card") && !strcmp(temp_user->login, login)) fprintf(users_temp, "%s\n", field);
        else fprintf(users_temp, "%s\n", temp_user->card);
        fprintf(users_temp, "%d\n", temp_user->is_admin);
    }
    fclose(users_txt);
    fclose(users_temp);

    rename("temp.txt", "users.txt");
}

void rename_favorites(const char *old_login, const char *new_login) {
    char old_filename[36] = "favorites/";
    strcat(old_filename, old_login);
    strcat(old_filename, ".txt");

    char new_filename[36] = "favorites/";
    strcat(new_filename, new_login);
    strcat(new_filename, ".txt");
    rename(old_filename, new_filename);
}

// Изменение логина
void change_login(User *user) {
    printf("Ваш текущий логин: %s. (Чтобы выйти из функции - введите \"Q\")\n", user->login);

    regex_t regex;
    if (regcomp(&regex, "^[[:alpha:][:digit:]]{3,20}$", REG_EXTENDED)) {
        fprintf(stderr, "Не удалось скомпилировать regex-выражение.\n");
        exit(1);
    }

    char field[24];
    while (1) {
        printf("Введите логин на который хотите изменить >> ");
        scanf("%s", field);
        if (!strcmp(user->login, field) || !strcmp(field, "Q")) {
            return;
        } else if (regexec(&regex, field, 0, NULL, 0) == REG_NOMATCH) {
            printf("Логин должен состоять из латинских букв и цифр. И его длина должна быть в диапазоне 3-20 символов.\n");
        } else if (check_field(field)) {
            printf("Такой логин уже используется.\n");
        } else {
            break;
        }
    }
    edit_field(user->login, "login", field);
    rename_favorites(user->login, field);
    strcpy(user->login, field);
}

// Изменение пароля
void change_password(User *user) {
    printf("Ваш текущий пароль: %s. (Чтобы выйти из функции - введите \"Q\")\n", user->password);

    char field[24];
    while (1) {
        printf("Введите новый пароль для аккаунта (от 6 до 20 символов) >> ");
        scanf("%s", field);
        if (!strcmp(field, "Q") || !strcmp(user->password, field)) return;
        int has_lowercase = 0, has_uppercase = 0, has_number = 0;
        for (int i = 0; i < strlen(field); i++) {
            if (islower(field[i])) has_lowercase++;
            if (isupper(field[i])) has_uppercase++;
            if (isdigit(field[i])) has_number++;
        }
        if (has_lowercase && has_uppercase && has_number && strlen(field) >= 6) break;
        else
            printf("Пароль должен содержать хотя бы одну латинскую букву в верхнем и нижнем регистре. "
                   "И его длина должна быть в диапазоне от 3 до 20 символов\n");
    }
    edit_field(user->login, "password", field);
    strcpy(user->password, field);
}

// Изменение номера карты
void change_card(User *user) {
    printf("Ваш текущий номер карты: %s. (Чтобы выйти из функции - введите \"Q\")\n", user->card);

    char field[24];
    while (1) {
        printf("Введите номер карты на который хотите сменить >> ");
        scanf("%s", field);
        if (check_luhn(field)) break;
        else printf("Карта невалидна.\n");
    }
    edit_field(user->login, "card", field);
    strcpy(user->card, field);
}

// Меню личного кабинета
void profile(User *user) {
    while (1) {
        system("clear");
        printf("╔ %s, добро пожаловать в личный кабинет. Здесь вы можете изменить данные профиля.\n", user->login);
        printf("╟ 1. Изменить имя пользователя.\n");
        printf("╟ 2. Сменить пароль.\n");
        printf("╟ 3. Изменить номер карты.\n");
        printf("╚ e. Выход.\n");

        system("/bin/stty raw");
        int ch = getchar();
        system("/bin/stty cooked");

        system("clear");
        if (ch == '1') {
            change_login(user);
        } else if (ch == '2') {
            change_password(user);
        } else if (ch == '3') {
            change_card(user);
        } else if (ch == 'e') {
            return;
        }
    }
}

// Добавление фильмов в режиме админа
void add_film_admin(Films *films) {
    system("clear");
    Film *film = add_film(films);

    char buffer[100];
    printf("Введите название фильма >> ");
    fgets(film->title, sizeof(film->title), stdin);

    while (1) {
        printf("Введите год фильма >> ");
        fgets(buffer, sizeof(buffer), stdin);
        film->year = atoi(buffer);
        if (film->year > 1900 && film->year < 2025) break;
    }

    printf("Введите страны в которых сделан фильм >> ");
    fgets(film->countries, sizeof(film->countries), stdin);

    printf("Введите жанры фильма >> ");
    fgets(film->genres, sizeof(film->genres), stdin);

    while (1) {
        printf("Введите рейтинг фильма >> ");
        fgets(buffer, sizeof(buffer), stdin);
        film->rating = atof(buffer);
        if (film->rating > 0 && film->rating < 10) break;
    }

    strtok(film->title, "\r\n");
    strtok(film->countries, "\r\n");
    strtok(film->genres, "\r\n");

    FILE *films_txt = fopen("films.txt", "a");
    write_film(films_txt, film);
    fclose(films_txt);
}

// Админ панель
void admin_panel(Films *films, User *user) {
    while (1) {
        system("clear");
        printf("╔ Добро пожаловать в меню навигации. Перейдите в нужный вам "
               "раздел.\n");
        printf("╟ 1. Добавить фильм.\n");
        printf("╟ 2. Удаление фильмов.\n");
        printf("╚ e. Выход.\n");

        int ch = input_mode();
        if (ch == '1') {
            add_film_admin(films);
        } else if (ch == '2') {
            show_films(films, user, 2);
        } else if (ch == 'e') {
            return;
        }
    }
}

// Меню навигации для пользователя
void navigation_menu(Films *films, User *user) {
    while (1) {
        printf("╔ %s, добро пожаловать в меню навигации. Перейдите в нужный вам раздел.\n", user->login);
        printf("╟ 1. Личный кабинет.\n");
        printf("╟ 2. Каталог всех фильмов.\n");
        printf("╟ 3. Каталог избранных фильмов.\n");
        if (user->is_admin) {
            printf("╟ 4. Админ-панель.\n");
            printf("╚ e. Выход.\n");
        } else {
            printf("╚ e. Выход.\n");
        }

        int ch = input_mode();
        char result;
        if (ch == '1') {
            profile(user);
        } else if (ch == '2') {
            result = show_films(films, user, 0);
        } else if (ch == '3') {
            result = show_films(user->favorites, user, 1);
        } else if (ch == '4' && user->is_admin) {
            admin_panel(films, user);
        } else if (ch == 'e') {
            return;
        }
        system("clear");
        if (ch == '2' && result) {
            printf("Пока что администратор не добавил ни одного фильма.\n");
        } else if (ch == '3' && result) {
            printf("У вас нет фильмов в списке избранных. Для начала добавьте их.\n");
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_col < 96 || w.ws_row < 28) {
        printf("Минимальное разрешение терминала для успешной работы 96x28.\n");
        return 0;
    }

    Films *films = get_films_from_file("films.txt");
    system("clear");

    User *user = auth();
    system("clear");

    navigation_menu(films, user);
    system("clear");

    print_gigachad();
    return 0;
}
