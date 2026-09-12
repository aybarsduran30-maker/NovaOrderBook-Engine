#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "OrderBook.hpp"

namespace py = pybind11;

PYBIND11_MODULE(nova_engine, m) {
    py::enum_<Side>(m, "Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL)
        .export_values();

    py::enum_<OrderType>(m, "OrderType")
        .value("LIMIT", OrderType::LIMIT)
        .value("IOC", OrderType::IOC)
        .export_values();

    py::class_<Trade>(m, "Trade")
        .def_readonly("buyOrderId", &Trade::buyOrderId)
        .def_readonly("sellOrderId", &Trade::sellOrderId)
        .def_readonly("price", &Trade::price)
        .def_readonly("count", &Trade::count)
        .def_readonly("timestamp", &Trade::timestamp);

    py::class_<BatchOrder>(m, "BatchOrder")
        .def(py::init<uint64_t, uint32_t, uint32_t, Side, OrderType>(),
             py::arg("id"), py::arg("price"), py::arg("count"), py::arg("side"), py::arg("type") = OrderType::LIMIT)
        .def_readwrite("id", &BatchOrder::id)
        .def_readwrite("price", &BatchOrder::price)
        .def_readwrite("count", &BatchOrder::count)
        .def_readwrite("side", &BatchOrder::side)
        .def_readwrite("type", &BatchOrder::type);

    py::class_<OrderBook>(m, "OrderBook")
        .def(py::init<>())
        .def("addOrder", &OrderBook::addOrder,
             py::arg("id"), py::arg("side"), py::arg("price"), py::arg("count"), py::arg("type") = OrderType::LIMIT)
        .def("addOrdersBatch", &OrderBook::addOrdersBatch, py::arg("orders"))
        .def("cancelOrder", &OrderBook::cancelOrder, py::arg("orderPoolIndex"))
        .def("cancelOrderById", &OrderBook::cancelOrderById, py::arg("orderId"))
        .def("getBestBid", &OrderBook::getBestBid)
        .def("getBestAsk", &OrderBook::getBestAsk)
        .def("getTradeCount", &OrderBook::getTradeCount);
}