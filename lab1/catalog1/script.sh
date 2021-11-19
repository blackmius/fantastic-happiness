#!/bin/bash

show_menu(){
    normal=`echo "\033[m"`
    menu=`echo "\033[36m"`
    number=`echo "\033[33m"`
    bgred=`echo "\033[41m"`
    fgred=`echo "\033[31m"`
    printf "\n${menu}*********************************************${normal}\n"
    printf "${menu}**${number} 1)${menu} Текущий пользователь ${normal}\n"
    printf "${menu}**${number} 2)${menu} Объем используемой памяти ${normal}\n"
    printf "${menu}**${number} 3)${menu} Объем дискового пространства ${normal}\n"
    printf "${menu}**${number} 4)${menu} Список запущенных процессов ${normal}\n"
    printf "${menu}**${number} 5)${menu} Список процессов, запущенных текущим пользователем ${normal}\n"
    printf "${menu}**${number} 6)${menu} Системные дата и время ${normal}\n"
    printf "${menu}**${number} 7)${menu} Время запуска системы ${normal}\n"
    printf "${menu}**${number} 8)${menu} Путь до текущего каталога ${normal}\n"
    printf "${menu}**${number} 9)${menu} moo ${normal}\n"
    printf "${menu}**${number} 10)${menu} История команд ${normal}\n"
    printf "${menu}**${number} 11)${menu} Проверить доступ к интернету ${normal}\n"
    printf "${menu}*********************************************${normal}\n"
    printf "Please enter a menu option and enter or ${fgred}x to exit. ${normal}"
    read opt
}

option_picked(){
    msgcolor=`echo "\033[01;31m"` # bold red
    normal=`echo "\033[00;00m"` # normal white
    message=${@:-"${normal}Error: No message passed"}
    printf "${msgcolor}${message}${normal}\n"
}

clear
show_menu
while [[ $opt != '' ]]
    do
    if [ $opt = '' ]; then
      exit;
    else
      case $opt in
        1) clear;
            option_picked "Выбран пункт 1";
            whoami;
            show_menu;
        ;;
        2) clear;
            option_picked "Выбран пункт 2";
            free -h;
            show_menu;
        ;;
        3) clear;
            option_picked "Выбран пункт 3";
            df -h;
            show_menu;
        ;;
        4) clear;
            option_picked "Выбран пункт 4";
            ps -a | less;
            show_menu;
        ;;
        5) clear;
            option_picked "Выбран пункт 5";
            ps -u $(whoami) | less;
            show_menu;
        ;;
        6) clear;
            option_picked "Выбран пункт 6";
            date;
            show_menu;
        ;;
        7) clear;
            option_picked "Выбран пункт 7";
            uptime;
            show_menu;
        ;;
        8) clear;
            option_picked "Выбран пункт 8";
            echo $(pwd);
            show_menu;
        ;;
        9) clear;
            option_picked "Выбран пункт 9";
            apt-get moo;
            show_menu;
        ;;
        10) clear;
            option_picked "Выбран пункт 10";
            less ~/.bash_history;
            show_menu;
        ;;
        11) clear;
            option_picked "Выбран пункт 11";
            ping 8.8.8.8
            show_menu;
        ;;
        x)exit;
        ;;
        \n)exit;
        ;;
        *)clear;
            option_picked "Выберите пункт из меню";
            show_menu;
        ;;
      esac
    fi
done
