import os
import pandas as pd

# Path to your feeder34 directory
directory = './feeder34'

print("Starting cleanup process...")

for filename in os.listdir(directory):
    old_path = os.path.join(directory, filename)

    # Skip if it's a directory
    if not os.path.isfile(old_path):
        continue

    # 1. Clean the filename: replace spaces with underscores, fix typos, make lowercase
    new_filename = filename.replace(' ', '_')
    new_filename = new_filename.replace('Feede_.doc', 'Feeder.doc') # Fix the typo
    new_filename = new_filename.lower() # Optional: makes filenames consistent

    # 2. Handle .xls to .csv conversion
    if new_filename.endswith('.xls'):
        csv_filename = new_filename.replace('.xls', '.csv')
        csv_path = os.path.join(directory, csv_filename)

        try:
            print(f"Converting: '{filename}' -> '{csv_filename}'")
            # Read the .xls file (requires xlrd installed)
            df = pd.read_excel(old_path, engine='xlrd')
            # Save it as a .csv file
            df.to_csv(csv_path, index=False)

            # Delete the old .xls file (Uncomment the line below if you want to delete the original .xls files)
            os.remove(old_path)

        except Exception as e:
            print(f"Error converting '{filename}': {e}")

    # 3. Handle non-xls files (just rename them)
    else:
        new_path = os.path.join(directory, new_filename)
        if old_path != new_path:
            print(f"Renaming: '{filename}' -> '{new_filename}'")
            os.rename(old_path, new_path)

print("Cleanup complete!")