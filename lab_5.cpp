#include <algorithm>
#include <iostream>
#include <vector>
#include <math.h>

static int y = 0;
static const unsigned long Y_VALUE_LIMIT = (unsigned long)1 << (sizeof(y) * 8 - 1);

// Генератор псевдослучайных чисел (равномерное распределение на отрезке [0, 1])
double rnd() {
    y *= 843314861;
    y += 453816693;
    if (y < 0) {
        y += Y_VALUE_LIMIT;
    }
    return double(y) / double(Y_VALUE_LIMIT - 1);
}

// Генератор псевдослучайных чисел (экспоненциальное распределение с параметром lambda > 0)
double esv(const double &lambda) {
    // Взятие модуля параметра для безопасности
    double parameter = abs(lambda);

    // Проверка на положительность параметра. В случае нуля установить параметр равным 1
    if (parameter == 0) {
        std::cerr << "Error! The parameter of exponential distribution equals 0!\n It will be set to 1!\n";
        parameter = 1;
    }

    // Возвращение случайного числа
    return -log(rnd()) / parameter;
}

// Класс Пуассоновского процесса
class PoissonPointProcess {
public:
    //Конструктор по интенсивности потока и началу отсчёта
    PoissonPointProcess(const double&, const double&);
    // Получение следующего момента, в который наступит событие
    double GetNextPoint();

private:
    // Интенсивность потока
    double intensity;
    // Текущий момент (в который наступило событие)
    double current_point;
};

// Конструктор Пуассоновского процесса (lambda - интенсивность потока, start - начало отсчёта)
PoissonPointProcess::PoissonPointProcess(const double &lambda, const double &start) {
    // Взятие модуля параметра для безопасности
    double parameter = abs(lambda);

    // Проверка на положительность параметра. В случае нуля установить параметр равным 1
    if (parameter == 0) {
        std::cerr << "Error! The intensity of Poisson point process equals 0!\n It will be set to 1!\n";
        parameter = 1;
    }

    // Установление интенсивности и начала отсчёта
    intensity = parameter;
    current_point = abs(start);
}

// Получение следующего момента, в который наступит событие
double PoissonPointProcess::GetNextPoint() {
    // Вычисление следующего момента по экспоненциальному распределению с заданным параметом intensity
    current_point += esv(intensity);

    // Возвращение следующего момента
    return current_point;
}

// Класс устройства, который обрабатывает некоторого рода запросы
class Device {
public:
    // Конструктор по интенсивности
    Device(const double&);
    // Обработка запроса по заданому моменту
    bool ProceedRequest(const double&);
    // Получение момента освобождения
    double GetMomentOfRelease();

private:
    // Интенсиврость потока
    double intensity;
    // Момент освобождения
    double moment_of_release;
};

// Конструктор устройства (mu - интенсивность)
Device::Device(const double &mu) {
    // Взятие модуля параметра для безопасности
    double parameter = abs(mu);

    // Проверка на положительность параметра. В случае нуля установить параметр равным 1
    if (parameter == 0) {
        std::cerr << "Error! The intensity of device equals 0!\n It will be set to 1!\n";
        parameter = 1;
    }

    // Установление интенсивности и момента освобождения
    intensity = parameter;
    moment_of_release = 0;
}

// Обработка запроса по заданому моменту (true - принять заявку, false - отклонить заявку)
bool Device::ProceedRequest(const double &moment) {
    // Если момент подачи заявки позже момента освобождения, то принять заявку, иначе отклонить.
    if (moment >= moment_of_release) {
        // Получение нового момента освобождения
        moment_of_release = moment + esv(intensity);
        // Принять заявку
        return true;
    }
    // Отклонить заявку
    return false;
}

// Получение момента освобождения
double Device::GetMomentOfRelease() {
    return moment_of_release;
}

// Класс интенсивности выходного потока
class StreamOutIntensity {
public:
    // Конструктор по начальному моменту
    StreamOutIntensity(const double&);
    // Функция обновления значения по заданному моменту
    void UpdateValue(const double&);
    // Функция получения значения
    double GetValue();

private:
    // Начальный момент
    double start_moment;
    // Последний момент
    double last_moment;
    // Итоговое значение интенсивности выходного потока
    double value;
};

// Конструктор интенсивности выходного потока (start - начальный момент)
StreamOutIntensity::StreamOutIntensity(const double &start) {
    // Установление начального и последнего моментов модулем поданного на вход момента
    start_moment = abs(start);
    last_moment = abs(start);
    // Установление значения интенсивности выходного потока нулю
    value = 0;
}

// Функция обновления значения (moment - следующий момент)
void StreamOutIntensity::UpdateValue(const double &moment) {
    // Взятие модуля момента для безопасности
    double new_moment = abs(moment);

    // Если следующий момент наступит раньше последнего, то не обновлять значение
    if (new_moment < last_moment) {
        return;
    }

    // Умножение значения на делитель для обновления
    value *= last_moment - start_moment;
    // Увеличение значения на 1 в связи с добавлением нового момента
    value++;
    // Установление последнего момента
    last_moment = new_moment;
    // Деление значения на требуемый делитель
    value /= last_moment - start_moment;
}

// Функция получения значения интенсивности выходного потока
double StreamOutIntensity::GetValue() {
    return value;
}

int main() {
    // Создание Пуассоновского потока с параметром lambda и начальным моментом = 0
    double lambda = 2;
    PoissonPointProcess process(lambda, 0);

    // Создание устройства с параметром mu
    double mu = 5;
    Device device(mu);

    // Установление правой границы временного отрезка
    double T = 100;
    // Получение первого момента
    double moment = process.GetNextPoint();
    //std::cout << "Moment: " << moment << '\n';

    // Если полученный момент находится за временным отрезком, то вернуть ошибку
    if (moment >= T) {
        return 1;
    }

    // Подать заявку с заданным моментом на устройство
    device.ProceedRequest(moment);
    //std::cout << "Request Accepted! Release moment: " << device.GetMomentOfRelease() << "\n";

    // Инициализация интенсивности выходного потока заявок первым моментом освобождения
    StreamOutIntensity request_out_intensity(device.GetMomentOfRelease());

    // Получение следующего момента
    moment = process.GetNextPoint();
    //std::cout << "Moment: " << moment << '\n';

    // Цикл по всему временному отрезку
    while (moment < T) {
        // Если заявка принимается, то обновляется значение интенсивности выходного потока заявок
        if (device.ProceedRequest(moment)) {
            //std::cout << "Request Accepted! Release moment: " << device.GetMomentOfRelease() << "\n";
            request_out_intensity.UpdateValue(device.GetMomentOfRelease());
        }
        // Получение следующего момента
        moment = process.GetNextPoint();
        //std::cout << "Moment: " << moment << '\n';
    }

    // Вывод результатов
    std::cout << "There has been immitated Poison point process of device's queue with parameters: lambda = " << lambda << ", mu = " << mu << ", T = " << T << ".\nRequest out stream intensity: " << request_out_intensity.GetValue() << "\n";
    return 0;
}
