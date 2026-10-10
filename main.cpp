#include <QApplication>
#include "back/simulation.h"
#include "internal/simulation_cli.h"
#include <iostream>
#include <exception>

int main(int argc, char *argv[])
{
    try {
        const auto options = model_detail::parse_command_line(argc, argv);
        if (options.help) {
            model_detail::print_help(std::cout);
            return 0;
        }
        // Консольный эксперимент запускается без доступа к графической сессии,
        // даже если переменные окружения экрана унаследованы от рабочего стола.
        if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
        QApplication a(argc, argv);
        configure_simulation(options.parameters);
        init_simulation();
        if (!options.quiet) model_detail::print_catalog(std::cout);
        for (int day = 1; day <= options.parameters.days; ++day) {
            manage_warehouse(day);
            generate_daily_events(day);
            process_daily_order(day);
            cleanup_random_customers();
            if (!options.quiet) model_detail::print_daily_report(std::cout, simulation_result(), options.show_orders);
        }
        print_final_statistics();
        if (!options.report_file.empty()) model_detail::write_json_report(options.report_file);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
