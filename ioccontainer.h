#ifndef IOCCONTAINER_H
#define IOCCONTAINER_H

#include <functional> //лямбда-выражения и функторы
#include <iostream>
#include <memory> //умные указатели
#include <map> //std::map, где хранятся фабрики объектов (ключ - ID, значение - фабрика)
#include <string>
#include <QString>
#include <QDebug>

using namespace std;

class IOCContainer
{
    static int s_nextTypeId; //статич переменная для следующего доступного ID для нового типа. Это нужно для хранения в map




    template<typename T> //шаблон, Т - заполнитель. Здесь это статический шаблонный метод внутри класса IOCContainer
    static int GetTypeID()
    {
        static int typeId = s_nextTypeId++;
        return typeId;
    }




public:

    class FactoryRoot //нужен, чтобы хранить разные фабрики в одной коллекции std::map. Их можно хранить как shared_ptr<FactoryRoot>
    {
    public:
        virtual ~FactoryRoot() {}
    };

    std::map<int, std::shared_ptr<FactoryRoot>> m_factories; //карта фабрик. контейнер, который хранит значения парами (ключ-значение)
    //Когда запрашивается объект типа T, контейнер ищет фабрику по ID типа T и вызывает её




    //Получить экземпляр объекта
    template<typename T>// Это шаблонный класс, который служит обёрткой для фабрики объектов типа T. Позволяет хранить любые фабрики в общей карте.
    class CFactory : public FactoryRoot
    {
        std::function<std::shared_ptr<T>()> m_functor; //хранит функтор, который создаёт объект типа Т. Функтор - это объект, который ведёт себя как функция. Это объект класса, у которого перегружен оператор operator().

    public:
        ~CFactory() {}

        CFactory(std::function<std::shared_ptr<T>()> functor)//принимает функтор и сохраняет его
            : m_functor(functor)
        {}

        std::shared_ptr<T> GetObject() { //вызывает сохранённый функтор и возвращает созданный объект
            return m_functor();
        }
    };

    //НОВАЯ фабрика для адаптеров
    template<typename T>
    class AdapterFactory : public FactoryRoot
    {
        std::function<bool(const QString&)> m_condition;
        std::function<std::shared_ptr<T>(const QString&)> m_functor;  // ← теперь принимает QString!

    public:
        AdapterFactory(std::function<bool(const QString&)> condition,
                       std::function<std::shared_ptr<T>(const QString&)> functor)
            : m_condition(condition), m_functor(functor) {}

        bool CheckCondition(const QString& p) const {
            return m_condition(p);
        }

        std::shared_ptr<T> GetObject(const QString& param) {  // ← теперь с параметром!
            return m_functor(param);
        }
    };



    template<typename T> //шаблонный метод внутри класса IOCContainer. Т - параметр шаблонного метода
    std::shared_ptr<T> GetObject() { //метод для получения объекта типа Т
        auto typeId = GetTypeID<T>(); //получаем объект типа Т
        auto factoryBase = m_factories[typeId]; //ищем в карте фабрику по ID
        auto factory = std::static_pointer_cast<CFactory<T>>(factoryBase); //приведение указатель на FactoryRoot(базовый класс) к указателю на CFactory<T>(конкретный класс)
        return factory->GetObject(); //вызов GetObject() из CFactory, который вызывает функтор и возвращает объект
    }

    //НОВОЕ получение объекта, исходя из условия
    template<typename T>
    std::shared_ptr<T> GetObject(const QString& param)
    {

        auto typeId = GetTypeID<T>();
        auto it = m_factories.find(typeId);

        if (it == m_factories.end()) {


            return nullptr;
        }

        auto factoryBase = it->second;


        // 1. Пробуем как AdapterFactory (условная фабрика с параметром)
        auto AFactory = std::dynamic_pointer_cast<AdapterFactory<T>>(factoryBase);
        if (AFactory) {

            bool conditionResult = AFactory->CheckCondition(param);


            if (conditionResult) {

                auto obj = AFactory->GetObject(param);  // ← передаём param в фабрику!

                return obj;
            } else {

                return nullptr;
            }
        }


        // 2. Пробуем как CFactory (обычная фабрика без параметра)
        auto factory = std::static_pointer_cast<CFactory<T>>(factoryBase);
        if (factory) {

            return factory->GetObject();
        }



            return nullptr;
    }




    //РЕГИСТРАЦИЯ ЭКЗЕМПЛЯРОВ

    //Регистрация через ФУНКТОР
    //Самая простая реализация - зарегистрировать функтор. Позволяет зарегистрировать фабрику, которая автоматически разрешает зависимости объекта
    template<typename TInterface, typename... TS> //шаблон с переменным числом параметров. TInterface - интерфейс, для которого регистрируется фабрика, TS - типы зависимостей, которые нужны для создания объекта
    void RegisterFunctor(
        std::function<std::shared_ptr<TInterface>(std::shared_ptr<TS>... ts)> functor) { //функтор, который создаёт объект TInterface, принимая зависимости TS
        m_factories[GetTypeID<TInterface>()]/*вставляем в карту фабрику для интерфейса*/ = std::make_shared<CFactory<TInterface>>( //создаём фабрику, которая при вызове GetObject() выполнит лямбду
            [ = ] { return functor(GetObject<TS>()...); }); //лямбда с захватом по значению [=]. Захват по значению создаёт копию functor, которая будет жить столько, сколько живёт лямбда. Ссылка [&] могла бы стать недействительной при уничтожении функтора
    } //лямбда вызывает переданный функтор, передавая ему зависимости через GetObject<TS>().


    //Регистрация ОДНОГО ЭКЗЕМПЛЯРА объекта
    //Позволяет зарегистрировать конкретный объект (синглтон), который уже существует. При каждом запросе GetObject<TInterface>() будет возвращаться один и тот же объект
    template<typename TInterface>
    void RegisterInstance(std::shared_ptr<TInterface> t) {// std::shared_ptr<TInterface> t - готовый экземпляр объекта
        m_factories[GetTypeID<TInterface>()]/*вставляем в карту фабрику для интерфейса*/ = std::make_shared<CFactory<TInterface>>(//создаём фабрику, которая при вызове GetObject() выполнит лямбду
            [ = ] { return t; });//лямбда с захватом по значению [=].Захват по значению создаёт копию t
    } //лямбда возвращает сохранённый объект

    //Подаем УКАЗАТЕЛЬ НА ФУНКЦИЮ
    //Позволяет передавать обычную функцию вместо функтора
    template<typename TInterface, typename... TS>
    void RegisterFunctor(std::shared_ptr<TInterface> (*functor)(std::shared_ptr<TS>... ts)) { //принимает указатель на функцию вместо функтора
        RegisterFunctor( //вызывает основной RegisterFunctor
            std::function<std::shared_ptr<TInterface>(std::shared_ptr<TS>... ts)>(functor)); //преобразует указатель в функтор
    }

    //ФАБРИКА, которая будет вызывать КОНСТРУКТОР, для КАЖДОГО ЭКЗЕМПЛЯРА
    //Позволяет зарегестрировать фабрику для объектов, создающихся через конструктор с параметрами-зависимостями
    template<typename TInterface, typename TConcrete, typename... TArguments> //объявление шаблонной функции
    //TInterface - интерфейс, который будет возвращаться
    //TConcrete - конкретный класс, который будет создаваться
    //TArguments - типы аргументов конструктора TConcrete (должны уже быть зарегестрированы в контейнере). Например, для IntelProcessor(double, ProcessorType, string) это будут double, ProcessorType, string
    void RegisterFactory() {
        RegisterFunctor( //принимает функтор и регистрирует его в контейнере. Мы "не создаём новый метод", м делегируем его задачу уже существующему RegisterFunctor
            std::function<std::shared_ptr<TInterface>(std::shared_ptr<TArguments>... ts)>(// создание функтора, возвращаемого интерфейс, принимающего параметры функтора, который будет выполнять лямбду
                []/*лямбда не захватывает переменные из внешнего кода*/(std::shared_ptr<TArguments>... arguments)/*параметры лямбды*/ -> std::shared_ptr<TInterface> { //стрелка указывает, что возвращает лямбда
                        //тело лямбды:
                        return std::make_shared<TConcrete>(//создаёт TConcrete и возвращает shared_ptr<TConcrete>
                        std::forward<std::shared_ptr<TArguments>>(arguments)...); // forward - идеальная передача, т к сохраняет lvalue и rvalue, которые при передачи аргументов могут быть неявно преобразованы, хотя функции подразумевают принятие аргумента без преобразования; TArguments - тип, который мы передаём; arguments - что м передаём (из аргументов лямбды)
                }));
    }

    //ФАБРИКА, которая будет возвращать ОДИН ЭКЗЕМПЛЯР
    //Позволяет регестрировать ещё несозданный объект, путём его создания внутри метода
    template<typename TInterface, typename TConcrete, typename... TArguments> //больше аргументов шаблона, чтоб можно бло создать объект
    void RegisterInstance() { //нет аргументов, т к объект ещё не создан
        RegisterInstance<TInterface>(std::make_shared<TConcrete>(GetObject<TArguments>()...));//получает все зависимости из контейнера GetObject<TArguments>()...; Создаёт TConcrete; Полученный TConcrete передаёт в прошлый RegisterInstance
    }//метод создаёт объект сам, в отличие от прошлого RegisterInstance

    //НОВАЯ фабрика с проверкой условия. Для возвращения нужного адаптера
    template<typename TInterface>
    void RegisterAdapter(
        std::function<bool(const QString&)> condition,
        std::function<std::shared_ptr<TInterface>(const QString&)> functor)  // ← functor с параметром!
    {
        m_factories[GetTypeID<TInterface>()] =
            std::make_shared<AdapterFactory<TInterface>>(condition, functor);
    }

};

extern IOCContainer gContainer; //это объявление вместо определения, которое может быть всего раз во всей программе, оно должно быть в cpp. Создаётся глобальный контейнер, доступный из любого файла проекта



#endif // IOCCONTAINER_H

