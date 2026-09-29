#include <vector>

// партия на складе
class StoreBatch {
    public:
        int mas_med_id; //индекс из mas_med
        int count;
        int expiration_day; //день симуляции когда истечет срок годности партии
        double batch_price; // цена за единицу товара

        double price(); // посчитать общую стоимость партии
        int take(int cnt); //забирает со склада cnt упаковок (если в партии меньше возвращает сколько есть)
};

// склад
class Warehouse{
    public:
        std::vector <StoreBatch> inventory;

        void write_off(); //списать просроченые партии в убыток и уценить товары с истекающим сроком годности
        bool need_restock(); // проверить упало ли кол-во каких либо лекарств меньше чем их min_stock 
};