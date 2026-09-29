//заявка поставщику
class Request {
    public:
        int mas_med_id;  // индекс лекарства из mas_med
        int count;
        int delivery_timer; // таймер доставки в днях (случайное от 1 до 3)
};