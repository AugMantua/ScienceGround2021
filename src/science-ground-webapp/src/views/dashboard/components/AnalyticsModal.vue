<template>
  <v-dialog
    v-model="isOpen"
    fullscreen
    hide-overlay
    transition="dialog-bottom-transition"
  >
    <v-card>
      <v-toolbar  height="35" dark color="primary">
        <v-btn icon dark @click="isOpen = false">
          <v-icon>mdi-close</v-icon>
        </v-btn>
        <v-toolbar-title class="pa-0">Andamento del terrario: {{terrariumName}}</v-toolbar-title>
      </v-toolbar>

      <v-row no-gutters>
         <!-- GRAFICI -->
        <v-col cols="2" v-if="!$vuetify.breakpoint.smAndDown">
            <timefilsers />
        </v-col>
        <!-- GRAFICI --><v-col :cols="!$vuetify.breakpoint.smAndDown ? '10' : '12'">
        <v-card  elevation="2" class="mt-3 ml-4 mr-4" >
         
          <v-container  >
              <v-tabs v-model="tabs"  center-active>
                 <v-tab>Temperatura</v-tab>
                 <v-tab>Umidità</v-tab>
                 <v-tab>Co2</v-tab>
                 <v-tab>Luminosità</v-tab>
              </v-tabs>

              <v-tabs-items v-model="tabs">
                 <v-tab-item>
                    <v-card outlined>
                    <apexchart 
                      type="line"
                      :options="options"
                      :series="series"
                    ></apexchart>
                  </v-card>
                </v-tab-item>
                 <v-tab-item>
                    <v-card outlined>
                    <apexchart
                      type="line"
                      :options="options"
                      :series="series"
                    ></apexchart>
                  </v-card>
                </v-tab-item>
                 <v-tab-item>
                    <v-card outlined>
                    <apexchart
                      type="line"
                      :options="options"
                      :series="series"
                    ></apexchart>
                  </v-card>
                </v-tab-item>
                 <v-tab-item>
                    <v-card outlined>
                    <apexchart
                      type="line"
                      :options="options"
                      :series="series"
                    ></apexchart>
                  </v-card>
                </v-tab-item>
              </v-tabs-items>

          </v-container>

       
        </v-card> </v-col>
      </v-row>
    </v-card>
  </v-dialog>
</template>

<script>
import { EventBus } from "../../../main";
import VueApexCharts from "vue-apexcharts";
import TimeFilsers from './TimeFilsers.vue';
export default {
  name: "AnalyticsModal",
  components: {
    apexchart: VueApexCharts,
    timefilsers: TimeFilsers
  },
  data() {
    return {
      isOpen: false,
      tabs: null,
      terrariumName:"",
      terrariumId:"",
     
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
    let self = this;
    console.log(this.$vuetify.breakpoint);
    EventBus.$on("changeDiaalogState", (value) => {
      self.isOpen = value.visibility;
      self.terrariumName = value.terrariumName;
      self.terrariumId = value.terrariumId;
    });
  },
};
</script>
