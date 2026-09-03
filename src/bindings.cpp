#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "OrderBook.hpp"

namespace py = pybind11;

PYBIND11_MODULE(nova_engine, m) {
    py::enum_<Side>(m, "Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL)
        .export_values();

    py::class_<Trade>(m, "Trade")
        .def_readonly("buyOrderId", &Trade::buyOrderId)
        .def_readonly("sellOrderId", &Trade::sellOrderId)
        .def_readonly("price", &Trade::price)
        .def_readonly("count", &Trade::count)
        .def_readonly("timestamp", &Trade::timestamp);

    py::class_<OrderBook>(m, "OrderBook")
        .def(py::init<>())
        .def("addOrder", &OrderBook::addOrder)
        .def("cancelOrder", &OrderBook::cancelOrder)
        .def("getBestBid", &OrderBook::getBestBid)
        .def("getBestAsk", &OrderBook::getBestAsk)
        .def("getTradeCount", &OrderBook::getTradeCount);
}