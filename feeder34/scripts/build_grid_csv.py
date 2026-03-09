import pandas as pd
import numpy as np
import os

directory = './feeder34'

# print("Building bus.csv and li.csv...")

# ==========================================
# 1. BUILD LINE.CSV (Branches)
# ==========================================
# Read the line data (skipping the top 2 garbage rows)
line_file = os.path.join(directory, 'line_data.csv')
lines_df = pd.read_csv(line_file, skiprows=2)

# Rename columns to be programming-friendly
lines_df.columns = ['from_bus', 'to_bus', 'length_ft', 'config']

# Clean up any empty rows that might have been read
lines_df = lines_df.dropna(subset=['from_bus', 'to_bus'])

# Convert bus IDs to integers
lines_df['from_bus'] = lines_df['from_bus'].astype(int)
lines_df['to_bus'] = lines_df['to_bus'].astype(int)

# Save line.csv
# lines_df.to_csv('line.csv', index=False)
# print("Created line.csv with", len(lines_df), "branches.")


# ==========================================
# 2. BUILD BUS.CSV (Nodes)
# ==========================================
# First, get a list of ALL unique buses from the line data
all_buses = set(lines_df['from_bus']).union(set(lines_df['to_bus']))
bus_df = pd.DataFrame({'bus_id': sorted(list(all_buses))})

# Set Default Bus Types (1 = PQ/Load, 2 = PV/Gen, 3 = Slack/Swing)
bus_df['bus_type'] = "PQ/Load"
# In IEEE 34, Bus 800 is the source (Slack Bus)
bus_df.loc[bus_df['bus_id'] == 800, 'bus_type'] = "Slack/Swing"

# --- Add Spot Loads ---
try:
    spot_load_file = os.path.join(directory, 'spot_load_data.csv')
    # Skip garbage rows (adjust skiprows if your file has more/less)
    loads_df = pd.read_csv(spot_load_file, skiprows=2)

    # Assuming columns are like: Node, Ph-1 kW, Ph-1 kVAr, Ph-2 kW...
    # We will rename the first column to 'bus_id' to match our bus_df
    loads_df.rename(columns={loads_df.columns[0]: 'bus_id'}, inplace=True)
    loads_df['bus_id'] = loads_df['bus_id'].astype(int)

    # Merge the load data into our main bus dataframe
    bus_df = pd.merge(bus_df, loads_df, on='bus_id', how='left')
except Exception as e:
    print(f"Note: Could not merge spot loads automatically. ({e})")

# --- Add Capacitors ---
try:
    cap_file = os.path.join(directory, 'cap_data.csv')
    caps_df = pd.read_csv(cap_file, skiprows=2)
    caps_df.rename(columns={caps_df.columns[0]: 'bus_id'}, inplace=True)
    caps_df['bus_id'] = caps_df['bus_id'].astype(int)

    # Merge the capacitor data
    bus_df = pd.merge(bus_df, caps_df, on='bus_id', how='left')
except Exception as e:
    print(f"Note: Could not merge capacitors automatically. ({e})")

# Replace NaN (Not a Number) values with 0 for buses that have no load/caps
bus_df = bus_df.fillna(0)

# Save bus.csv
# bus_df.to_csv('bus.csv', index=False)
# print("Created bus.csv with", len(bus_df), "buses.")
# print("Done!")