#pragma once
// Подключаем заголовочный файл для работы с агентами.
#include <sc-memory/sc_agent.hpp>
// Создаём класс, реализующий агент вычисления булеана множества.
// Наследуем его от класса ScActionInitiatedAgent, описывающего
// класс агентов, реагирующих на инициированное действие.
class CalculatingDegreeOfVertices : public ScActionInitiatedAgent
{
public:
    // Объявляем метод, возвращающий sc-адрес класса действий,
    // выполняемых указанным агентом.
    ScAddr GetActionClass() const override;
    // Объявляем метод, реализующий основную логику указанного агента
    // – выполняющий инициированное действие и возвращающий результат
    // выполнения действия.
    ScResult DoProgram(ScAction & action) override;
};
