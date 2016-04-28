#include "common/conversion.h"
#include "order_serialization.h"
#include "lib/seq_write.h"

static void WriterOrder(simulation_order Order, seq_write *W) {
  ui16 PlayerIDUI8 = SafeCastIntToUI8(Order.PlayerID);

  SeqWriteUI8(W, PlayerIDUI8);
  SeqWriteUI16(W, Order.UnitCount);
  SeqWriteSI16(W, Order.Target.X);
  SeqWriteSI16(W, Order.Target.Y);
  for(memsize I=0; I<Order.UnitCount; ++I) {
    ui16 UnitIDUI16 = SafeCastIntToUI16(Order.UnitIDs[I]);
    SeqWriteUI16(W, UnitIDUI16);
  }
}

static simulation_order ReadOrder(serializer *S, linear_allocator *Allocator) {
  simulation_order Order;
  Order.PlayerID = SerializerReadUI8(S);
  Order.UnitCount = SerializerReadUI16(S);
  Order.Target.X = SerializerReadSI16(S);
  Order.Target.Y = SerializerReadSI16(S);

  memsize UnitIDsSize = sizeof(simulation_unit_id) * Order.UnitCount;
  Order.UnitIDs = (simulation_unit_id*)LinearAllocate(Allocator, UnitIDsSize);

  for(memsize I=0; I<Order.UnitCount; ++I) {
    Order.UnitIDs[I] = SerializerReadSI16(S);
  }

  return Order;
}

buffer SerializeOrder(simulation_order Order, linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);
  WriterOrder(Order, &W);
  return W.Buffer;
}

simulation_order UnserializeOrder(buffer OrderBuffer, linear_allocator *Allocator) {
  serializer W = CreateSerializer(OrderBuffer);
  return ReadOrder(&W, Allocator);
}

buffer SerializeOrderList(simulation_order_list *List, linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);
  SeqWriteUI16(&W, List->Count);

  for(memsize I=0; I<List->Count; ++I) {
    WriterOrder(List->Orders[I], &W);
  }

  return W.Buffer;
}

simulation_order_list UnserializeOrderList(buffer ListBuffer, linear_allocator *Allocator) {
  serializer R = CreateSerializer(ListBuffer);
  simulation_order_list List;
  List.Count = SerializerReadUI16(&R);

  if(List.Count != 0) {
    memsize OrdersSize = sizeof(simulation_order) * List.Count;
    List.Orders = (simulation_order*)LinearAllocate(Allocator, OrdersSize);

    for(memsize I=0; I<List.Count; ++I) {
      List.Orders[I] = ReadOrder(&R, Allocator);
    }
  }
  else {
    List.Orders = NULL;
  }

  return List;
}
