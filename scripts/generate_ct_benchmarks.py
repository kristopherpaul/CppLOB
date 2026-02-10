#!/usr/bin/env python3
"""
Generate compile-time benchmark configurations.
"""
import os
import sys

def generate_configs():
    configs = []
    
    # Scaling dimensions
    orders_per_side_values = [1, 2, 5, 10, 20, 50, 100]
    price_levels_per_side_values = [1, 2, 5, 10, 20]
    
    for ops in orders_per_side_values:
        for pls in price_levels_per_side_values:
            if ops < pls:
                continue  # Skip invalid: more levels than orders
            
            # Generate bid levels (descending prices)
            bid_levels = []
            bid_book_parts = []
            for i in range(pls):
                price = 100 - i
                orders = []
                for j in range(ops // pls + (1 if i < ops % pls else 0)):
                    order_id = i * 100 + j + 1
                    orders.append(f"Order<Side::Buy, {price}, 10, {order_id}>")
                if orders:
                    level = f"PriceLevel<{price}, {', '.join(orders)}>"
                    bid_levels.append(level)
                    bid_book_parts.append(level)
            
            # Generate ask levels (ascending prices)
            ask_levels = []
            ask_book_parts = []
            for i in range(pls):
                price = 101 + i
                orders = []
                for j in range(ops // pls + (1 if i < ops % pls else 0)):
                    order_id = 1000 + i * 100 + j + 1
                    orders.append(f"Order<Side::Sell, {price}, 10, {order_id}>")
                if orders:
                    level = f"PriceLevel<{price}, {', '.join(orders)}>"
                    ask_levels.append(level)
                    ask_book_parts.append(level)
            
            if not bid_book_parts or not ask_book_parts:
                continue
            
            config = {
                'name': f'ops{ops}_pls{pls}',
                'orders_per_side': ops,
                'price_levels_per_side': pls,
                'bid_levels': ',\n    '.join(bid_levels),
                'ask_levels': ',\n    '.join(ask_levels),
                'bid_book': f"BidBook<{', '.join(bid_book_parts)}>",
                'ask_book': f"AskBook<{', '.join(ask_book_parts)}>",
                'incoming_price': 101,
                'incoming_qty': 50,
                'incoming_id': 9999,
            }
            configs.append(config)
    
    return configs

def render_template(template_path, config, output_path):
    with open(template_path, 'r') as f:
        template = f.read()
    
    replacements = {
        '@ORDERS_PER_SIDE@': str(config['orders_per_side']),
        '@PRICE_LEVELS_PER_SIDE@': str(config['price_levels_per_side']),
        '@BID_LEVELS@': config['bid_levels'],
        '@ASK_LEVELS@': config['ask_levels'],
        '@BID_BOOK@': config['bid_book'],
        '@ASK_BOOK@': config['ask_book'],
        '@INCOMING_PRICE@': str(config['incoming_price']),
        '@INCOMING_QTY@': str(config['incoming_qty']),
        '@INCOMING_ID@': str(config['incoming_id']),
    }
    
    for k, v in replacements.items():
        template = template.replace(k, v)
    
    with open(output_path, 'w') as f:
        f.write(template)

def main():
    template_path = sys.argv[1] if len(sys.argv) > 1 else 'benchmarks/compile_time/compile_time_scaling.cpp.in'
    output_dir = sys.argv[2] if len(sys.argv) > 2 else 'benchmarks/compile_time/generated'
    
    os.makedirs(output_dir, exist_ok=True)
    
    configs = generate_configs()
    print(f"Generating {len(configs)} compile-time benchmark configurations...")
    
    for config in configs:
        output_path = os.path.join(output_dir, f"compile_time_scaling_{config['name']}.cpp")
        render_template(template_path, config, output_path)
        print(f"  Generated: {output_path}")

if __name__ == '__main__':
    main()