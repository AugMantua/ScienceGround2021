<template>
  <v-dialog
    v-model="isOpen"
    fullscreen
    hide-overlay
    transition="dialog-bottom-transition"
  >
    <v-card>
      <v-toolbar dark color="primary">
        <v-btn icon dark @click="isOpen = false">
          <v-icon>mdi-close</v-icon>
        </v-btn>
        <v-toolbar-title>Settings</v-toolbar-title>
      </v-toolbar>

      <v-row no-gutters>
         <!-- GRAFICI -->
        <v-col cols="3" v-if="!$vuetify.breakpoint.smAndDown">
          <v-card outlined class="mt-3 ml-1" elevation="1"> 
            <v-card-title outlined class="ma-0 pa-0">Filtri</v-card-title>
            <v-card outlined class="ma-2">
            <div class="ma-2">
              <v-menu
                ref="fromDate"
                v-model="fromPickerVis"
                :close-on-content-click="false"
                :return-value.sync="dateFrom"
                transition="scale-transition"
                offset-y
                min-width="290px"
              >
                <template v-slot:activator="{ on, attrs }">
                  <v-text-field
                    v-model="dateFrom"
                    label="Data di partenza"
                    readonly
                    v-bind="attrs"
                    v-on="on"
                  ></v-text-field>
                </template>
                <v-date-picker v-model="dateFrom" no-title scrollable>
                  <v-spacer></v-spacer>
                  <v-btn text color="primary" @click="fromPickerVis = false"
                    >Cancel</v-btn
                  >
                  <v-btn text color="primary" @click="$refs.fromDate.save(dateFrom)"
                    >OK</v-btn
                  >
                </v-date-picker>
              </v-menu>
            </div>   
            <div class="ma-2">
              <v-menu
                ref="timeFromP"
                v-model="fromTimePick"
                :close-on-content-click="false"
                :nudge-right="40"
                :return-value.sync="timeFrom"
                transition="scale-transition"
                offset-y
                max-width="290px"
                min-width="290px"
              >
                <template v-slot:activator="{ on, attrs }">
                  <v-text-field
                    v-model="timeFrom"
                    label="Ora di partenza"
                    readonly
                    v-bind="attrs"
                    v-on="on"
                  ></v-text-field>
                </template>
                <v-time-picker
                  v-if="fromTimePick"
                  v-model="timeFrom"
                  full-width
                  @click:minute="$refs.timeFromP.save(timeFrom)"
                ></v-time-picker>
              </v-menu>
            </div>
         <!--
          </v-card>
             <v-card outlined class="ma-2">
            <div class="ma-2">
              <v-menu
                ref="menu"
                v-model="menu"
                :close-on-content-click="false"
                :return-value.sync="date"
                transition="scale-transition"
                offset-y
                min-width="290px"
              >
                <template v-slot:activator="{ on, attrs }">
                  <v-text-field
                    v-model="date"
                    label="Data di fine"
                    readonly
                    v-bind="attrs"
                    v-on="on"
                  ></v-text-field>
                </template>
                <v-date-picker v-model="date" no-title scrollable>
                  <v-spacer></v-spacer>
                  <v-btn text color="primary" @click="menu = false"
                    >Cancel</v-btn
                  >
                  <v-btn text color="primary" @click="$refs.menu.save(date)"
                    >OK</v-btn
                  >
                </v-date-picker>
              </v-menu>
            </div>
            <div class="ma-2">
              <v-menu
                ref="menu"
                v-model="menu2"
                :close-on-content-click="false"
                :nudge-right="40"
                :return-value.sync="time"
                transition="scale-transition"
                offset-y
                max-width="290px"
                min-width="290px"
              >
                <template v-slot:activator="{ on, attrs }">
                  <v-text-field
                    v-model="time"
                    label="Ora di fine"
                    readonly
                    v-bind="attrs"
                    v-on="on"
                  ></v-text-field>
                </template>
                <v-time-picker
                  v-if="menu2"
                  v-model="time"
                  full-width
                  @click:minute="$refs.menu.save(time)"
                ></v-time-picker>
              </v-menu>
            </div> -->
          </v-card>
         
          </v-card>
        </v-col>
        <!-- GRAFICI -->
        <v-col>
          <v-container fluid>
            <v-row no-gutters>
              <v-col>
                <v-card outlined>
                  <v-card-title class="ma-0 pa-0">Temperatura</v-card-title>
                  <apexchart
                    type="line"
                    :options="options"
                    :series="series"
                  ></apexchart>
                </v-card>
              </v-col>
              <v-col>
                <v-card outlined tile>
                  <v-card-title class="ma-0 pa-0">Umidità</v-card-title>
                  <apexchart
                    type="line"
                    :options="options"
                    :series="series"
                  ></apexchart>
                </v-card>
              </v-col>

              <v-responsive width="100%"></v-responsive>
            </v-row>
            <v-row no-gutters>
              <v-col>
                <v-card outlined>
                  <v-card-title class="ma-0 pa-0">Co2</v-card-title>
                  <apexchart
                    type="line"
                    :options="options"
                    :series="series"
                  ></apexchart>
                </v-card>
              </v-col>
              <v-col>
                <v-card outlined>
                  <v-card-title class="ma-0 pa-0">Luminosità</v-card-title>
                  <apexchart
                    type="line"
                    :options="options"
                    :series="series"
                  ></apexchart>
                </v-card>
              </v-col>
              <v-responsive width="100%"></v-responsive>
            </v-row>
          </v-container>
        </v-col>
      </v-row>
    </v-card>
  </v-dialog>
</template>

<script>
import { EventBus } from "../../../main";
import VueApexCharts from "vue-apexcharts";
export default {
  name: "AnalyticsModal",
  components: {
    apexchart: VueApexCharts,
  },
  data() {
    return {
      isOpen: false,
      dateFrom: new Date().toISOString().substr(0, 10),
      fromPickerVis: false,
      timeFrom: null,
      fromTimePick: false,
      options: {
        chart: {
          id: "vuechart-example",
        },
        xaxis: {
          categories: [1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998],
        },
      },
      series: [
        {
          name: "series-1",
          data: [30, 40, 45, 50, 49, 60, 70, 91],
        },
      ],
    };
  },

  mounted() {
    console.log(this.$vuetify.breakpoint);
    EventBus.$on("changeDiaalogState", (value) => {
      this.isOpen = value;
    });
  },
};
</script>
