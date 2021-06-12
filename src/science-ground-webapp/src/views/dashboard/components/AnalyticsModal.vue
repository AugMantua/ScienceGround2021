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

      <v-container  fluid >
        <v-row no-gutters>
          <v-col>
            <v-card  outlined tile>
              <apexchart
                type="line"
                :options="options"
                :series="series"
              ></apexchart>
            </v-card>
          </v-col>
          <v-col>
            <v-card outlined tile>
              <apexchart
                type="line"
                :options="options"
                :series="series"
              ></apexchart>
            </v-card>
          </v-col>
          <v-responsive width="100%" ></v-responsive>
        </v-row>
         <v-row no-gutters>
          <v-col>
            <v-card  outlined tile>
              <apexchart
                type="line"
                :options="options"
                :series="series"
              ></apexchart>
            </v-card>
          </v-col>
          <v-col>
            <v-card  outlined tile>
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
    EventBus.$on("changeDiaalogState", (value) => {
      this.isOpen = value;
    });
  },
};
</script>
