//Лекартсва
class Medicine {
    public:
        int dosage;  //дозировка
        int type_med_id; //индекс из med_id
        int group_med_id;  // индекс из group_med
        double wholesale_price; // оптовая цена за 1 шт
        int shelf_life; // срок годности в днях
        int min_stock; // минимальный остаток на складе для формирования заявки на пополнение
};