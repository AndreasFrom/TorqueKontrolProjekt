import csv
from datetime import datetime

def split_csv_by_header(filename, header_prefix='timestamp'):
    output_file_index = 0
    current_rows = []
    header = None

    run_time = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

    with open(filename, 'r', newline='') as csvfile:
        reader = csv.reader(csvfile)

        for row in reader:
            if row and row[0].strip().lower() == header_prefix:
                if current_rows:
                    output_filename = f'output_{run_time}_{output_file_index}.csv'
                    with open(output_filename, 'w', newline='') as outfile:
                        writer = csv.writer(outfile)
                        writer.writerows(current_rows)
                    output_file_index += 1
                    current_rows = []

                header = row
                current_rows.append(header)
            else:
                current_rows.append(row)

        if current_rows:
            output_filename = f'output_{run_time}_{output_file_index}.csv'
            with open(output_filename, 'w', newline='') as outfile:
                writer = csv.writer(outfile)
                writer.writerows(current_rows)

split_csv_by_header('v075-IT.csv')
