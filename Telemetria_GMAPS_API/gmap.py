import gmaps
from gmaps import locations
import gmaps.datasets
from gmaps.heatmap import heatmap_layer

import api
# autenticando credenciais
gmaps.configure(api_key = api.api_key)

# dataset de terremotos 
earthquake_df = gmaps.datasets.load_dataset_as_df('earthquakes')
print('Coordenadas de Terremotos:')
print(earthquake_df.head())

# definicao de coordenadas dos terremotos
locations = earthquake_df[['latitude', 'longitude']]
weights = earthquake_df['magnitude']
fig = gmaps.figure()
fig.add_layer(gmaps.heatmap_layer(locations, weights = weights))
fig

# exibindo mapa em modo satelite
fig = gmaps.figure(map_type= 'SATELLITE')
fig

# exibindo localizacoes especificas
locations = [(51.5, 0.1), (51.7, 0.2), (51.4, -0.2), (51.49, 0.1)]
heatmap_layer = gmaps.heatmap_layer(locations)
fig.add_layer(heatmap_layer)

# definindo coordenadas de Gaspar
gaspar_coordinates = (-26.93, -48.95)
fig = gmaps.figure(center = gaspar_coordinates, zoom_level = 12, map_type= 'HYBRID')
fig
