#include <QApplication>

#include "back/customer.h"
#include "back/data.h"
#include "back/medicine.h"
#include "back/order.h"
#include "back/request.h"
#include "back/statistics.h"
#include "back/store.h"
#include "back/data_generator.h"

// инициализация ввод N M K наценки создание постоянных клиентов и начального запаса склада
void init_simulation();

//генерация заказов на сегодня добавление разовых клиентов в массив и проверка расписания постоянных
void generate_daily_events(int current_day);

// выполнение заказов курьеры развозят товары склад списывает проданное обновляеться статистика
void process_daily_order(int current_day); 

// работа со складом проверка сроков годности и если нужно дозакупка
void manage_warehouse(int current_day);

// удаление не постоянных клиентов из масива коиентов (в конце дня)
void cleanup_random_customers();

// вывод финального отчета
void print_final_statistics();


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    return a.exec();
}
