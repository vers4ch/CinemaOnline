//
//  main.c
//  CinemaOnline_NEW
//
//  Created by Danil Degtyarev on 16.12.2022.
//
#include <stdio.h>
#include <stdlib.h>

//create let for MovieCard
typedef struct movie_card{     //struct for MovieCard
    int id;                   //id movie
    char *title;             //name movie
    int year;               //year movie
    char *country;         //contry recording
    char *genre;          //genre movie
    float rating;        //rating movie
    struct movie_card *next, *prev;
}movie_card, mc;


mc *init(int id, char *title, int year, char *country, char *genre, float rating){// New ONLY START element
    mc *main;
    main = (mc*)malloc(sizeof(mc));// выделение памяти под мост списка
    main->id = id;
    main->title = title;
    main->year = year;
    main->country = country;
    main->genre = genre;
    main->rating = rating;
    main->next = NULL; // указатель на следующий узел
    main->prev = NULL; // указатель на предыдущий узел
    return(main);
}

mc *addElement(mc *lst, int id, char *title, int year, char *country, char *genre, float rating){ //Adding Elem
    mc *card, *p;
    card = (mc*)malloc(sizeof(mc));
    p = lst->next; // сохранение указателя на следующий узел
    lst->next = card; // предыдущий узел указывает на создаваемый
    //adding data to uzel
    card->id = id;
    card->title = title;
    card->year = year;
    card->country = country;
    card->genre = genre;
    card->rating = rating;
    //point to next/prev element
    card->next = p; // созданный узел указывает на следующий узел
    card->prev = lst; // созданный узел указывает на предыдущий узел
    if (p != NULL)
        p->prev = card;
    return(card);
}

mc * delElement(mc *lst){  //Deleting element
    mc *prev, *next;
    prev = lst->prev; // узел, предшествующий lst
    next = lst->next; // узел, следующий за lst
    if (prev != NULL)
        prev->next = lst->next; // переставляем указатель
    if (next != NULL)
        next->prev = lst->prev; // переставляем указатель
    free(lst); // освобождаем память удаляемого элемента
    return(prev);
}

mc *deletAll(mc *root){  //clean RAM
    mc *temp;
    temp = root->next;
    temp->prev = NULL;
    free(root);   // освобождение памяти текущего моста
    return(temp); // новый мост списка
}

void listprint(mc *lst){  //Printing card
    mc *p;
    p = lst;
    do {
        printf("ID: %d\nTitle: %s\nYear: %d\nCountry: %s\nGenre: %s\nRating: %.1f\n\n", p->id, p->title, p->year, p->country, p->genre, p->rating); // вывод значения элемента p
        p = p->next; // переход к следующему узлу
    }
    while (p != NULL); // условие окончания обхода
}

void listprintr(mc *lst){
    mc *p;
    p = lst;
    while (p->next != NULL)
        p = p->next;  // переход к концу списка
    do {
        printf("ID: %d\nTitle: %s\nYear: %d\nCountry: %s\nGenre: %s\nRating: %.1f\n\n", p->id, p->title, p->year, p->country, p->genre, p->rating); // вывод значения элемента p
        p = p->prev; // переход к предыдущему узлу
    }
    while (p != NULL); // условие окончания обхода
}

void step(mc *lst){
    mc *p;
    p = lst;
    while (p->next != NULL)
        p = p->next;  // переход к концу списка
    for (int i = 0; i<1; i++){
        printf("ID: %d\nTitle: %s\nYear: %d\nCountry: %s\nGenre: %s\nRating: %.1f\n\n", p->id, p->title, p->year, p->country, p->genre, p->rating); // вывод значения элемента p
        p = p->prev; // переход к предыдущему узлу
    }
}

int main(){
    char key;
    mc *card = init(0, "Форрест Гамп", 1994, "США", "драма, мелодрама", 8.9);
    addElement(card, 1, "Валли", 2008, "США", "Мультфильмы", 9.2);
    addElement(card, 2, "Список Шиндлера", 1993, "США", "драма, биография", 8.8);
    printf("==============\n");
    while (1) {
        scanf("%c", &key);
        if(key=='a'){
            system("clear");
            listprint(card);
        }
        if(key=='d'){
            system("clear");
            step(card);
        }
    }
    return 0;
}